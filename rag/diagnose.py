#!/usr/bin/env python3
"""诊断脚本：分析RAG系统的错误类型，找出优化方向"""

import json
import sys
from typing import List, Dict

def load_json(filepath: str) -> List[Dict]:
    with open(filepath, "r", encoding="utf-8") as f:
        return json.load(f)

def diagnose(output_file: str, answer_file: str):
    outputs = load_json(output_file)
    answers = load_json(answer_file)
    
    # 创建答案索引
    answer_map = {a["question"]: a for a in answers}
    
    # 统计
    total = len(outputs)
    filename_correct = 0
    page_correct = 0
    both_correct = 0
    
    errors = {
        "filename_wrong": [],      # filename错误
        "page_wrong": [],          # filename对但page错
        "both_correct": [],        # 都对
    }
    
    print("=" * 80)
    print("RAG 系统诊断报告")
    print("=" * 80)
    
    for out in outputs:
        q = out["question"]
        if q not in answer_map:
            print(f"[警告] 问题未在答案中找到: {q[:50]}...")
            continue
        
        ans = answer_map[q]
        
        filename_match = out["filename"] == ans["filename"]
        page_match = out["page"] == ans["page"]
        
        if filename_match:
            filename_correct += 1
            if page_match:
                page_correct += 1
                both_correct += 1
                errors["both_correct"].append({
                    "question": q,
                    "output": out,
                    "answer": ans,
                })
            else:
                errors["page_wrong"].append({
                    "question": q,
                    "output": out,
                    "answer": ans,
                })
        else:
            errors["filename_wrong"].append({
                "question": q,
                "output": out,
                "answer": ans,
            })
    
    # 输出统计
    print(f"\n📊 总体统计 (共 {total} 题)")
    print("-" * 40)
    print(f"✅ filename 正确: {filename_correct}/{total} ({filename_correct/total*100:.1f}%)")
    print(f"✅ page 正确 (在filename正确的基础上): {page_correct}/{filename_correct if filename_correct else 1} ({page_correct/max(filename_correct,1)*100:.1f}%)")
    print(f"✅ 两者都正确: {both_correct}/{total} ({both_correct/total*100:.1f}%)")
    
    # 预估分数
    # 假设 answer_score 平均 0.8
    estimated_score = (filename_correct/total * 25) + (page_correct/total * 25) + 0.8 * 50
    print(f"\n📈 预估分数: {estimated_score:.1f} (假设answer_score=0.8)")
    
    # 详细错误分析
    print("\n" + "=" * 80)
    print("❌ FILENAME 错误的题目 (丢失50分: 25 filename + 25 page)")
    print("=" * 80)
    for i, e in enumerate(errors["filename_wrong"][:10], 1):  # 只显示前10个
        print(f"\n[{i}] 问题: {e['question']}")
        print(f"    输出: filename='{e['output']['filename']}', page={e['output']['page']}")
        print(f"    正确: filename='{e['answer']['filename']}', page={e['answer']['page']}")
        print(f"    输出答案: {e['output']['answer'][:100]}...")
    
    if len(errors["filename_wrong"]) > 10:
        print(f"\n    ... 还有 {len(errors['filename_wrong']) - 10} 个filename错误")
    
    print("\n" + "=" * 80)
    print("⚠️  PAGE 错误的题目 (filename对但page错，丢失25分)")
    print("=" * 80)
    for i, e in enumerate(errors["page_wrong"][:10], 1):
        print(f"\n[{i}] 问题: {e['question']}")
        print(f"    输出: page={e['output']['page']}")
        print(f"    正确: page={e['answer']['page']}")
        print(f"    文件: {e['output']['filename']}")
    
    if len(errors["page_wrong"]) > 10:
        print(f"\n    ... 还有 {len(errors['page_wrong']) - 10} 个page错误")
    
    # 优化建议
    print("\n" + "=" * 80)
    print("💡 优化建议")
    print("=" * 80)
    
    filename_error_rate = len(errors["filename_wrong"]) / total
    page_error_rate = len(errors["page_wrong"]) / max(filename_correct, 1)
    
    if filename_error_rate > 0.3:
        print(f"\n🔴 filename 错误率较高 ({filename_error_rate*100:.1f}%)")
        print("   建议:")
        print("   1. 检查检索是否召回了正确的文档")
        print("   2. 检查LLM是否正确选择了来源")
        print("   3. 可能需要增加 TOP_K 扩大召回范围")
    
    if page_error_rate > 0.3:
        print(f"\n🟡 page 错误率较高 ({page_error_rate*100:.1f}%)")
        print("   建议:")
        print("   1. 检查分块是否把同一知识点切到了不同页")
        print("   2. 检查LLM是否正确识别了页码")
        print("   3. 可能需要调整 chunk 大小")
    
    return errors


if __name__ == "__main__":
    output_file = sys.argv[1] if len(sys.argv) > 1 else "./data/output.json"
    answer_file = sys.argv[2] if len(sys.argv) > 2 else "./data/answer.json"
    
    diagnose(output_file, answer_file)
