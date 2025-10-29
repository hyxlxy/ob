import json
import os
import select
import socket
import subprocess
import time
from typing import Any, Optional, Dict
from typing import List

from langchain_core.documents import Document
from langchain_core.embeddings import Embeddings
from langchain_core.vectorstores import VST, VectorStore
from langflow.base.vectorstores.model import (
    LCVectorStoreComponent,
    check_cached_vector_store,
)
from langflow.base.vectorstores.vector_store_connection_decorator import (
    vector_store_connection,
)
from langflow.helpers import docs_to_data
from langflow.inputs import StrInput, IntInput, FloatInput
from langflow.io import HandleInput
from langflow.schema.data import Data
from langflow.serialization import serialize


class MiniOBException(Exception):
    """Base exception class for MiniOB operations."""

    def __init__(
        self, message: str = None, operation: str = None, details: Dict[str, Any] = None
    ) -> None:
        self.operation = operation
        self.details = details or {}

        if message:
            full_message = message
        else:
            full_message = "MiniOB operation failed"

        if operation:
            full_message = f"[{operation}] {full_message}"

        if details:
            detail_str = ", ".join([f"{k}={v}" for k, v in details.items()])
            full_message = f"{full_message} (Details: {detail_str})"

        super().__init__(full_message)


class MiniOBConnectionException(MiniOBException):
    """Exception raised when connection to MiniOB server fails."""

    def __init__(
        self,
        message: str = None,
        server_address: str = None,
        server_port: int = None,
        server_socket: str = None,
        error_code: str = None,
    ) -> None:
        details = {}
        if server_address:
            details["server_address"] = server_address
        if server_port:
            details["server_port"] = server_port
        if server_socket:
            details["server_socket"] = server_socket
        if error_code:
            details["error_code"] = error_code

        super().__init__(
            message or "Failed to connect to MiniOB server",
            operation="CONNECTION",
            details=details,
        )


class MiniOBQueryException(MiniOBException):
    """Exception raised when SQL query execution fails."""

    def __init__(
        self,
        message: str = None,
        sql: str = None,
        result: str = None,
        execution_time: float = None,
    ) -> None:
        details = {}
        if sql:
            details["sql"] = sql[:200] + "..." if len(sql) > 200 else sql
        if result:
            details["result"] = result
        if execution_time:
            details["execution_time_seconds"] = execution_time

        super().__init__(
            message or "SQL query execution failed",
            operation="QUERY_EXECUTION",
            details=details,
        )


class MiniOBDataException(MiniOBException):
    """Exception raised when data processing fails."""

    def __init__(
        self,
        message: str = None,
        data_type: str = None,
        expected_format: str = None,
        actual_format: str = None,
        row_count: int = None,
    ) -> None:
        details = {}
        if data_type:
            details["data_type"] = data_type
        if expected_format:
            details["expected_format"] = expected_format
        if actual_format:
            details["actual_format"] = actual_format
        if row_count is not None:
            details["row_count"] = row_count

        super().__init__(
            message or "Data processing failed",
            operation="DATA_PROCESSING",
            details=details,
        )


class MiniOBTimeoutException(MiniOBException):
    """Exception raised when operations timeout."""

    def __init__(
        self,
        message: str = None,
        timeout_seconds: float = None,
        operation_type: str = None,
    ) -> None:
        details = {}
        if timeout_seconds:
            details["timeout_seconds"] = timeout_seconds
        if operation_type:
            details["operation_type"] = operation_type

        super().__init__(
            message or "Operation timed out", operation="TIMEOUT", details=details
        )


class MiniObConnector(object):
    def __init__(
        self,
        server_address: str,
        server_port: int,
        server_socket: str,
        time_limit: float,
        charset: str,
        log_func=None,
    ):
        if server_port < 0 or server_port > 65535:
            raise MiniOBConnectionException(
                message=f"Invalid server port: {server_port}",
                server_address=server_address,
                server_port=server_port,
                error_code="INVALID_PORT",
            )

        self.log_func = log_func or (lambda msg: None)

        self.__server_address = server_address
        self.__server_port = server_port
        self.__server_socket = os.getenv("MINIOB_SERVER_SOCKET") or server_socket or ""
        self.__buffer_size = 8192
        self.__charset = charset
        self.__socket = None

        self.__time_limit = time_limit
        if not self.__time_limit:
            self.__time_limit = 10.0

        self.log_func(f"Initializing MiniOB connector...")
        try:
            if len(self.__server_socket) > 0:
                self.log_func(f"Using Unix socket: {self.__server_socket}")
                sock = self.__init_unix_socket(self.__server_socket)
            else:
                self.log_func(f"Using TCP connection: {server_address}:{server_port}")
                sock = self.__init_tcp_socket(self.__server_address, self.__server_port)

            self.__socket = sock
            if sock is not None:
                self.log_func("Socket connection established successfully")
                self.__socket.setblocking(False)

                self.__poller = select.poll()
                self.__poller.register(
                    self.__socket,
                    select.POLLIN | select.POLLPRI | select.POLLHUP | select.POLLERR,
                )
                self.log_func("Socket poller configured successfully")
            else:
                self.log_func("Failed to establish socket connection")
                raise MiniOBConnectionException(
                    message="Socket connection returned None",
                    server_address=server_address,
                    server_port=server_port,
                    server_socket=server_socket,
                    error_code="NULL_SOCKET",
                )
        except Exception as e:
            if isinstance(e, MiniOBConnectionException):
                raise
            else:
                raise MiniOBConnectionException(
                    message=f"Failed to initialize connection: {str(e)}",
                    server_address=server_address,
                    server_port=server_port,
                    server_socket=server_socket,
                    error_code="INIT_FAILED",
                ) from e

    @staticmethod
    def __init_tcp_socket(server_address: str, server_port: int):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect((server_address, server_port))
            return s
        except socket.error as e:
            raise MiniOBConnectionException(
                message=f"Failed to establish TCP connection: {str(e)}",
                server_address=server_address,
                server_port=server_port,
                error_code="TCP_CONNECTION_FAILED",
            ) from e
        except Exception as e:
            raise MiniOBConnectionException(
                message=f"Unexpected error during TCP connection: {str(e)}",
                server_address=server_address,
                server_port=server_port,
                error_code="TCP_UNEXPECTED_ERROR",
            ) from e

    @staticmethod
    def __init_unix_socket(server_socket: str):
        try:
            sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            sock.connect(server_socket)
            return sock
        except socket.error as e:
            raise MiniOBConnectionException(
                message=f"Failed to establish Unix socket connection: {str(e)}",
                server_socket=server_socket,
                error_code="UNIX_SOCKET_CONNECTION_FAILED",
            ) from e
        except Exception as e:
            raise MiniOBConnectionException(
                message=f"Unexpected error during Unix socket connection: {str(e)}",
                server_socket=server_socket,
                error_code="UNIX_SOCKET_UNEXPECTED_ERROR",
            ) from e

    def __recv_response(
        self,
        timeout: float,
        total_timeout_seconds: int,
    ) -> str:
        result = b""

        if not timeout:
            timeout = self.__time_limit

        if timeout is not None:
            timeout *= 1000

        deadline = time.time() + 3600 * 24
        if total_timeout_seconds is not None and total_timeout_seconds > 0:
            deadline = time.time() + total_timeout_seconds

        while time.time() < deadline:
            events = self.__poller.poll(timeout)
            if len(events) == 0:
                raise MiniOBTimeoutException(
                    message=f"Poll timeout after {timeout / 1000} second(s)",
                    timeout_seconds=timeout / 1000,
                    operation_type="SOCKET_POLL",
                )

            (_, event) = events[0]
            if event & (select.POLLHUP | select.POLLERR):
                error_details = {
                    "POLLHUP": bool(event & select.POLLHUP),
                    "POLLERR": bool(event & select.POLLERR),
                    "event_code": event,
                }
                msg = (
                    f"Failed to receive from server. poll return "
                    f"POLLHUP={str(event & select.POLLHUP)} or POLLERR={str(event & select.POLLERR)}"
                )
                self.log_func(msg)
                raise MiniOBConnectionException(
                    message="Socket connection error during polling",
                    error_code="SOCKET_POLL_ERROR",
                )

            data = self.__socket.recv(self.__buffer_size)
            if len(data) > 0:
                try:
                    if data[0] == 0 and len(result) == 0:
                        self.log_func("Error: receive from server \\0 byte")
                        ps_ef = subprocess.run(
                            ["ps", "-ef"],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                            universal_newlines=True,
                        )

                        self.log_func(f"Process status stdout: {str(ps_ef.stdout)}")
                        self.log_func(f"Process status stderr: {str(ps_ef.stderr)}")
                except BaseException as ex:
                    self.log_func(f"Exception during data processing: {ex}")

                result += data
                self.log_func(
                    f"Received from server (size={len(data)}): {data[:100]}..."
                )

                if data.endswith(b"\0"):

                    result = result[:-1]
                    try:
                        decoded_result = result.decode(encoding=self.__charset)
                        self.log_func(
                            f"Successfully decoded response with length {len(decoded_result)}"
                        )
                        return decoded_result.strip() + "\n"
                    except UnicodeDecodeError as e:
                        self.log_func(f"Unicode decode error: {e}")

                        decoded_result = result.decode(
                            encoding=self.__charset, errors="replace"
                        )
                        self.log_func(
                            f"Decoded with error handling, length: {len(decoded_result)}"
                        )
                        return decoded_result.strip() + "\n"
            else:
                self.log_func(f"Error: receive from server returned zero length data")
                raise MiniOBConnectionException(
                    message="Received zero length data from server - connection may be closed",
                    error_code="ZERO_LENGTH_DATA",
                )

        if time.time() >= deadline:
            self.log_func(
                f"Timeout: receive from server timeout after {total_timeout_seconds} seconds"
            )

        raise MiniOBTimeoutException(
            message=f"Response timeout after {total_timeout_seconds} second(s)",
            timeout_seconds=total_timeout_seconds,
            operation_type="RECEIVE_RESPONSE",
        )

    def exec(
        self,
        sql: str,
        timeout=None,
        total_timeout_seconds=None,
    ) -> str:
        if not sql:
            raise MiniOBQueryException(
                message="SQL query cannot be empty or None", sql=sql or "None"
            )

        start_time = time.time()
        self.log_func(f"Executing SQL (length={len(sql)})")

        try:
            data = str.encode(sql, self.__charset)
            
            # For non-blocking socket, we need to send data in chunks
            total_sent = 0
            data_with_null = data + b"\0"
            
            while total_sent < len(data_with_null):
                try:
                    sent = self.__socket.send(data_with_null[total_sent:])
                    if sent == 0:
                        raise MiniOBConnectionException(
                            message="Socket connection broken during send",
                            error_code="SEND_FAILED"
                        )
                    total_sent += sent
                except BlockingIOError:
                    # Socket buffer is full, wait a bit
                    time.sleep(0.01)
                    continue
                    
            self.log_func(f"SQL command sent to server (size={len(data_with_null)})")
            self.log_func("SQL command sent to server, waiting for response...")

            result = self.__recv_response(timeout, total_timeout_seconds)
            execution_time = time.time() - start_time

            self.log_func(f"SQL response received from server: '{result[:100]}...'")
            self.log_func(f"SQL execution completed in {execution_time:.3f}s")

            result_lower = result.lower().strip()
            if (
                result_lower.startswith("error")
                or "failed" in result_lower
                or "exception" in result_lower
            ):
                raise MiniOBQueryException(
                    message="SQL execution returned error result",
                    sql=sql,
                    result=result,
                    execution_time=execution_time,
                )

            return result.strip("\n")

        except (MiniOBConnectionException, MiniOBTimeoutException):

            raise
        except Exception as e:
            execution_time = time.time() - start_time
            if isinstance(e, MiniOBQueryException):
                raise
            else:
                raise MiniOBQueryException(
                    message=f"Unexpected error during SQL execution: {str(e)}",
                    sql=sql[:200] + "..." if len(sql) > 200 else sql,
                    execution_time=execution_time,
                ) from e

    def close(self):
        if self.__socket:
            self.log_func("Closing MiniOB connector socket...")
            self.__socket.close()
            self.__socket = None
            self.log_func("MiniOB connector socket closed successfully")
        else:
            self.log_func("No socket to close")


class MiniOBVectorStore(VectorStore):
    def __init__(
        self,
        embedding: Embeddings | None,
        server_address: str,
        server_port: int,
        server_socket: str,
        time_limit: float,
        charset: str,
        log_func=None,
    ) -> None:
        self.__embedding: Embeddings | None = embedding
        self.__log_func = log_func or (lambda msg: None)

        self.connector: MiniObConnector = MiniObConnector(
            server_address=server_address,
            server_port=server_port,
            server_socket=server_socket,
            time_limit=time_limit,
            charset=charset,
            log_func=self.__log_func,
        )

        if not self.__embedding:
            raise MiniOBException(
                message="Embedding model is required but was not provided",
                operation="INITIALIZATION",
            )

        # Hardcode dimension for bge-m3 to avoid Ollama validation issues
        # bge-m3 always uses 1024 dimensions
        self.__embedding_dimension = 1024
        self.__log_func(f"Using hardcoded dimension for bge-m3: {self.__embedding_dimension}")
        
        # Verify the embedding model works (optional validation)
        try:
            test_text = "This is a test sentence for embedding dimension detection."
            self.__log_func(f"Validating embedding model with text: '{test_text}'")
            test_embedding = self.__embedding.embed_query(test_text)
            actual_dimension = len(test_embedding)
            if actual_dimension != self.__embedding_dimension:
                self.__log_func(f"Warning: Expected dimension {self.__embedding_dimension}, got {actual_dimension}")
                self.__embedding_dimension = actual_dimension
            self.__log_func(f"Embedding model validated successfully, dimension: {self.__embedding_dimension}")
        except Exception as e:
            # If validation fails, still use the hardcoded dimension
            error_msg = str(e)
            self.__log_func(f"Warning: Embedding validation failed: {error_msg}")
            self.__log_func(f"Continuing with hardcoded dimension: {self.__embedding_dimension}")

        # TODO: initialize miniob data
        self.__table_name = "rag_documents"
        self.__init_table()

    def __init_table(self) -> None:
        """Initialize the MiniOB table for storing documents and embeddings."""
        try:
            # Drop existing table first (MiniOB doesn't support IF NOT EXISTS)
            drop_table_sql = f"DROP TABLE IF EXISTS {self.__table_name}"
            self.__log_func(f"Dropping table if exists: {self.__table_name}")
            try:
                self.connector.exec(drop_table_sql)
            except:
                pass  # Table might not exist
            
            # Create table with document content, metadata, and embedding vector
            # Note: MiniOB doesn't support PRIMARY KEY AUTO_INCREMENT
            create_table_sql = f"""
            CREATE TABLE {self.__table_name} (
                id INT,
                content TEXT,
                metadata TEXT,
                embedding VECTOR({self.__embedding_dimension})
            )
            """
            self.__log_func(f"Creating table: {self.__table_name}")
            self.connector.exec(create_table_sql)
            self.__log_func(f"Table {self.__table_name} created successfully")
            
            # Initialize ID counter
            self.__next_id = 1
        except Exception as e:
            self.__log_func(f"Error initializing table: {str(e)}")
            raise MiniOBException(
                message=f"Failed to initialize MiniOB table: {str(e)}",
                operation="TABLE_INITIALIZATION",
                details={"table_name": self.__table_name},
            ) from e

    def similarity_search(
        self,
        query: str,
        k: int = 4,
        search_method: str = "Vector Search",
    ) -> list[Document]:
        self.__log_func(f"Performing similarity search for query: '{query}' with k={k}")
        result = []

        # TODO: search similar data from miniob
        try:
            # Generate embedding for the query
            query_embedding = self.__embedding.embed_query(query)
            query_embedding_str = json.dumps(query_embedding)

            # Search for similar documents using L2 distance
            search_sql = f"""
            SELECT id, content, metadata, L2_DISTANCE(embedding, '{query_embedding_str}') as distance
            FROM {self.__table_name}
            ORDER BY distance
            LIMIT {k}
            """
            self.__log_func(f"Executing similarity search SQL")
            search_result = self.connector.exec(search_sql)
            self.__log_func(f"Search result: {search_result}")

            # Parse the results and create Document objects
            if search_result and search_result.strip():
                lines = search_result.strip().split('\n')
                # Skip header line if present
                for line in lines[1:] if len(lines) > 1 else lines:
                    if line.strip():
                        try:
                            # Parse the result line
                            parts = line.split('\t')
                            if len(parts) >= 3:
                                content = parts[1]
                                metadata_str = parts[2] if len(parts) > 2 else "{}"
                                try:
                                    metadata = json.loads(metadata_str) if metadata_str else {}
                                except:
                                    metadata = {}
                                result.append(Document(page_content=content, metadata=metadata))
                        except Exception as e:
                            self.__log_func(f"Error parsing result line: {line}, error: {str(e)}")
        except Exception as e:
            self.__log_func(f"Error during similarity search: {str(e)}")
            raise MiniOBException(
                message=f"Failed to perform similarity search: {str(e)}",
                operation="SIMILARITY_SEARCH",
                details={"query": query[:100], "k": k},
            ) from e

        self.__log_func(f"Similarity search completed. Found {len(result)} documents")
        return result

    def add_documents(
        self,
        documents: list[Document],
        **kwargs: Any,
    ) -> List[str]:
        if not documents or len(documents) == 0:
            self.__log_func("No documents provided for insertion")
            return []

        self.__log_func(f"Processing {len(documents)} documents for insertion")

        # Filter out documents with empty page_content to avoid Ollama validation errors
        valid_documents = []
        empty_count = 0
        for i, doc in enumerate(documents):
            # Check if page_content exists and is not empty after stripping
            if hasattr(doc, 'page_content') and doc.page_content is not None:
                content = str(doc.page_content).strip()
                if content and len(content) > 0:
                    valid_documents.append(doc)
                else:
                    empty_count += 1
                    if empty_count <= 5:  # Only log first 5 empty documents
                        self.__log_func(f"Warning: Skipping document {i} with empty page_content after strip (original length: {len(str(doc.page_content))})")
            else:
                empty_count += 1
                if empty_count <= 5:
                    self.__log_func(f"Warning: Skipping document {i} with no page_content attribute or None value")

        if empty_count > 5:
            self.__log_func(f"Warning: Skipped {empty_count - 5} more empty documents (total empty: {empty_count})")

        if not valid_documents:
            self.__log_func("Warning: No valid documents to insert after filtering")
            return []

        self.__log_func(f"Filtered to {len(valid_documents)} valid documents (removed {empty_count} empty documents)")

        # Double-check: ensure all page_contents are non-empty strings
        page_contents: List[str] = []
        final_valid_documents = []
        for doc in valid_documents:
            content = str(doc.page_content).strip()
            if content and len(content) > 0:
                page_contents.append(content)
                final_valid_documents.append(doc)
            else:
                self.__log_func(f"Warning: Found empty content in supposedly valid document, skipping")

        if len(page_contents) != len(valid_documents):
            self.__log_func(f"Warning: Additional filtering removed {len(valid_documents) - len(page_contents)} documents")
            valid_documents = final_valid_documents

        if not page_contents:
            self.__log_func("Error: No valid page_contents after double-check filtering")
            return []

        self.__log_func(f"Final document count for embedding: {len(page_contents)}")

        # Final validation: check for any empty strings in page_contents
        self.__log_func("Validating page_contents before embedding generation...")
        invalid_indices = []
        for i, content in enumerate(page_contents):
            if not content or not isinstance(content, str) or len(content.strip()) == 0:
                invalid_indices.append(i)
                if len(invalid_indices) <= 3:
                    self.__log_func(f"ERROR: Found invalid content at index {i}: type={type(content)}, value='{content[:50] if content else 'None'}'")

        if invalid_indices:
            error_msg = f"Found {len(invalid_indices)} invalid (empty or non-string) contents in page_contents list"
            self.__log_func(f"ERROR: {error_msg}")
            self.__log_func(f"Invalid indices: {invalid_indices[:10]}")
            raise MiniOBDataException(
                message=error_msg,
                data_type="page_contents",
                expected_format="non-empty strings",
                actual_format=f"{len(invalid_indices)} invalid entries",
                row_count=len(page_contents),
            )

        self.__log_func(f"Validation passed: all {len(page_contents)} page_contents are valid non-empty strings")
        self.__log_func("Generating embeddings for documents...")
        try:
            embeddings: List[List[float]] = self.__embedding.embed_documents(
                page_contents
            )
        except Exception as e:
            raise MiniOBException(
                message=f"Failed to generate embeddings for documents: {str(e)}",
                operation="GENERATE_EMBEDDINGS",
                details={"document_count": len(page_contents)},
            ) from e

        if len(embeddings) != len(page_contents):
            self.__log_func(
                f"Error: Embedding count mismatch. Expected {len(page_contents)}, got {len(embeddings)}"
            )
            raise MiniOBDataException(
                message="Embedding count mismatch with document count",
                data_type="embeddings",
                expected_format=f"{len(page_contents)} embeddings",
                actual_format=f"{len(embeddings)} embeddings",
                row_count=len(page_contents),
            )

        # TODO: insert data into miniob
        inserted_ids = []
        batch_size = 10  # Smaller batch size to avoid too large SQL statements
        try:
            # Insert documents in batches
            for batch_start in range(0, len(valid_documents), batch_size):
                batch_end = min(batch_start + batch_size, len(valid_documents))
                batch_documents = valid_documents[batch_start:batch_end]
                batch_embeddings = embeddings[batch_start:batch_end]
                
                self.__log_func(f"Processing batch {batch_start // batch_size + 1}/{(len(valid_documents) + batch_size - 1) // batch_size}")
                
                # Insert documents one by one (MiniOB batch INSERT has bugs)
                for i, (document, embedding) in enumerate(zip(batch_documents, batch_embeddings)):
                    content = document.page_content
                    metadata = document.metadata or {}
                    # Handle metadata that's already a string (from serialize())
                    if isinstance(metadata, str):
                        metadata_str = metadata
                    else:
                        metadata_str = json.dumps(metadata)
                    embedding_str = json.dumps(embedding)

                    # Escape single quotes and remove newlines/tabs (MiniOB doesn't support them)
                    content_escaped = content.replace("'", "''").replace("\n", " ").replace("\t", " ")
                    metadata_escaped = metadata_str.replace("'", "''").replace("\n", " ").replace("\t", " ")
                    
                    # Manual ID management
                    doc_id = self.__next_id
                    self.__next_id += 1
                    
                    # Single-row INSERT
                    insert_sql = f"""
                    INSERT INTO {self.__table_name} (id, content, metadata, embedding)
                    VALUES ({doc_id}, '{content_escaped}', '{metadata_escaped}', '{embedding_str}')
                    """
                    result = self.connector.exec(insert_sql)
                    inserted_ids.append(str(doc_id))
                
                self.__log_func(f"Inserted batch {batch_start // batch_size + 1} ({len(batch_documents)} documents)")

            self.__log_func(f"Successfully inserted {len(inserted_ids)} documents in {(len(valid_documents) + batch_size - 1) // batch_size} batches")
        except Exception as e:
            self.__log_func(f"Error inserting documents: {str(e)}")
            raise MiniOBException(
                message=f"Failed to insert documents into MiniOB: {str(e)}",
                operation="INSERT_DOCUMENTS",
                details={"document_count": len(valid_documents), "inserted_count": len(inserted_ids)},
            ) from e

        return inserted_ids

    @classmethod
    def from_texts(
        cls: type[VST],
        texts: list[str],
        embedding: Embeddings,
        metadatas: Optional[list[dict]] = None,
        *,
        ids: Optional[list[str]] = None,
        **kwargs: Any,
    ) -> VST:
        raise NotImplementedError()


@vector_store_connection
class MiniOBVectorStoreComponent(LCVectorStoreComponent):
    display_name = "MiniOB"
    description = "miniob vector store component."
    documentation: str = "https://oceanbase.github.io/miniob/"
    icon = "database"
    name = "MiniOB"

    _cached_vector_store: MiniOBVectorStore | None = None

    inputs = [
        StrInput(
            name="server_address",
            display_name="Server Address",
            advanced=True,
            value="127.0.0.1",
        ),
        IntInput(
            name="server_port",
            display_name="Server port",
            advanced=True,
            value=6789,
        ),
        StrInput(
            name="server_socket",
            display_name="Server socket",
            advanced=True,
        ),
        IntInput(
            name="number_of_results",
            display_name="Number of Search Results",
            advanced=True,
            value=4,
        ),
        FloatInput(
            name="time_limit",
            display_name="Search Time Limit",
            advanced=True,
            value=10.0,
        ),
        StrInput(
            name="charset",
            display_name="Charset",
            advanced=True,
            value="utf-8",
        ),
        HandleInput(
            name="embedding_model",
            display_name="Embedding Model",
            input_types=["Embeddings"],
            info="",
            required=True,
            show=True,
        ),
        *LCVectorStoreComponent.inputs,
    ]

    outputs = [
        *LCVectorStoreComponent.outputs,
    ]
    
    def _already_build(self) -> bool:
        # TODO: check if the document has been built
        try:
            # Check if the table has any documents
            check_sql = f"SELECT COUNT(*) FROM {self._MiniOBVectorStoreComponent__table_name if hasattr(self, '_MiniOBVectorStoreComponent__table_name') else 'rag_documents'}"
            # This method is in the component, not the vector store, so we need to check differently
            # For now, return False to always rebuild
            return False
        except:
            return False

    def _add_documents_to_vector_store(self, vector_store: MiniOBVectorStore) -> None:
        self.log("Preparing to add documents to vector store...")
        self.ingest_data = self._prepare_ingest_data()

        documents = []
        for _input in self.ingest_data or []:
            if isinstance(_input, Data):
                documents.append(_input.to_lc_document())
            else:
                msg = "Vector Store Inputs must be Data objects."
                self.log(
                    f"Error: Invalid input type {type(_input)}, expected Data object"
                )
                raise TypeError(msg)

        self.log(f"Prepared {len(documents)} documents for ingestion")

        documents = [
            Document(
                page_content=doc.page_content,
                metadata=serialize(doc.metadata, to_str=True),
            )
            for doc in documents
        ]

        if documents:
            self.log(f"Adding {len(documents)} documents to the Vector Store.")
            try:
                vector_store.add_documents(documents)
                self.log(
                    f"Successfully added {len(documents)} documents to Vector Store"
                )
            except Exception as e:
                self.log(f"Error adding documents to Vector Store: {str(e)}")
                raise
        else:
            self.log("No documents to add to the Vector Store.")
            return None

    @check_cached_vector_store
    def build_vector_store(self) -> VectorStore:
        self.log("Building MiniOB Vector Store...")
        self.log(f"Connecting to server: {self.server_address}:{self.server_port}")

        self.log(
            f"Configuration: server_address={self.server_address}, server_port={self.server_port}"
        )
        self.log(
            f"Configuration: server_socket={self.server_socket}, time_limit={self.time_limit}"
        )
        self.log(
            f"Configuration: charset={self.charset}, embedding_model={type(self.embedding_model).__name__ if self.embedding_model else 'None'}"
        )

        vector_store = MiniOBVectorStore(
            embedding=self.embedding_model if self.embedding_model else None,
            server_address=self.server_address if self.server_address else "127.0.0.1",
            server_port=self.server_port if self.server_port else 6789,
            server_socket=self.server_socket if self.server_socket else "",
            time_limit=self.time_limit if self.time_limit else 10.0,
            charset=self.charset if self.charset else "utf-8",
            log_func=self.log,
        )

        self.log("Vector Store created successfully")
        if not self._already_build():
            self._add_documents_to_vector_store(vector_store)
        self.log("Vector Store build completed")
        return vector_store

    def search_documents(self, vector_store=None) -> list[Data]:
        if not vector_store:
            vector_store = self.build_vector_store()

        query = (self.search_query if self.search_query else "").strip()
        if not query:
            self.log("Warning: Empty search query provided")
            return []

        self.log(f"Searching for documents with query: '{query}'")
        self.log(
            f"Number of results requested: {self.number_of_results if self.number_of_results else 4}"
        )

        vector_store = vector_store or self.build_vector_store()

        data = docs_to_data(
            vector_store.search(
                query=query,
                search_type="similarity",
                k=self.number_of_results if self.number_of_results else 4,
            )
        )

        if not data:
            self.log(f"No documents found for query: '{query}'")
            return []

        self.log(f"Found {len(data)} documents for query: '{query}'")
        return data
