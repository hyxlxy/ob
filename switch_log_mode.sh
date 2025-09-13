#!/bin/bash

# Script to switch between debug and production log modes
# Usage: ./switch_log_mode.sh [debug|production|status]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_DIR="$SCRIPT_DIR/etc"
CURRENT_CONFIG="$CONFIG_DIR/observer.ini"
DEBUG_CONFIG="$CONFIG_DIR/observer-debug.ini"
PRODUCTION_CONFIG="$CONFIG_DIR/observer-production.ini"

show_usage() {
    echo "Usage: $0 [debug|production|status|clean]"
    echo ""
    echo "Commands:"
    echo "  debug      - Switch to debug mode (detailed logging)"
    echo "  production - Switch to production mode (minimal logging)"
    echo "  status     - Show current log mode"
    echo "  clean      - Clean up old log files"
    echo ""
    echo "Examples:"
    echo "  $0 debug      # Enable debug logging for troubleshooting"
    echo "  $0 production # Switch back to minimal logging"
    echo "  $0 clean      # Remove old log files to free disk space"
}

get_current_mode() {
    if [ ! -f "$CURRENT_CONFIG" ]; then
        echo "unknown"
        return
    fi
    
    local log_level=$(grep "^LOG_FILE_LEVEL=" "$CURRENT_CONFIG" | cut -d'=' -f2)
    case "$log_level" in
        1) echo "production" ;;
        4) echo "debug" ;;
        *) echo "custom" ;;
    esac
}

show_status() {
    local mode=$(get_current_mode)
    echo "Current log mode: $mode"
    
    if [ -f "$CURRENT_CONFIG" ]; then
        local file_level=$(grep "^LOG_FILE_LEVEL=" "$CURRENT_CONFIG" | cut -d'=' -f2)
        local console_level=$(grep "^LOG_CONSOLE_LEVEL=" "$CURRENT_CONFIG" | cut -d'=' -f2)
        echo "Log file level: $file_level"
        echo "Console level: $console_level"
    fi
    
    # Show log file sizes
    echo ""
    echo "Current log files:"
    ls -lh observer.log* 2>/dev/null || echo "No log files found"
}

switch_to_debug() {
    if [ ! -f "$DEBUG_CONFIG" ]; then
        echo "Error: Debug config file not found: $DEBUG_CONFIG"
        exit 1
    fi
    
    cp "$DEBUG_CONFIG" "$CURRENT_CONFIG"
    echo "Switched to DEBUG mode"
    echo "- Log level: DEBUG (4)"
    echo "- Console level: WARN (2)"
    echo "- Debug modules enabled for expression and vector components"
    echo ""
    echo "Note: This will generate more detailed logs. Remember to switch back to production mode when done debugging."
}

switch_to_production() {
    if [ ! -f "$PRODUCTION_CONFIG" ]; then
        echo "Error: Production config file not found: $PRODUCTION_CONFIG"
        exit 1
    fi
    
    cp "$PRODUCTION_CONFIG" "$CURRENT_CONFIG"
    echo "Switched to PRODUCTION mode"
    echo "- Log level: ERROR (1)"
    echo "- Console level: ERROR (1)"
    echo "- Minimal logging to save disk space"
}

clean_logs() {
    echo "Cleaning up old log files..."
    
    # Count and size before cleanup
    local count=$(ls observer.log.* 2>/dev/null | wc -l)
    local size=$(du -sh observer.log.* 2>/dev/null | awk '{total+=$1} END {print total}' || echo "0")
    
    if [ "$count" -gt 0 ]; then
        echo "Found $count old log files"
        rm -f observer.log.*
        echo "Removed $count old log files"
    else
        echo "No old log files to clean"
    fi
    
    # Also clean miniob log directory if it exists
    if [ -d "miniob/log" ]; then
        local miniob_logs=$(find miniob/log -name "*.log" 2>/dev/null | wc -l)
        if [ "$miniob_logs" -gt 0 ]; then
            find miniob/log -name "*.log" -delete
            echo "Cleaned $miniob_logs files from miniob/log directory"
        fi
    fi
    
    echo "Log cleanup completed"
}

# Main script logic
case "${1:-status}" in
    debug)
        switch_to_debug
        ;;
    production)
        switch_to_production
        ;;
    status)
        show_status
        ;;
    clean)
        clean_logs
        ;;
    help|--help|-h)
        show_usage
        ;;
    *)
        echo "Error: Unknown command '$1'"
        echo ""
        show_usage
        exit 1
        ;;
esac
