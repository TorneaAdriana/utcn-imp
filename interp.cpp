// This file is part of the IMP project.

#include "interp.h"
#include "program.h"

#include <iostream>



// -----------------------------------------------------------------------------
void Interp::Run()
{
  for (;;) {
    auto op = prog_.Read<Opcode>(pc_);
    switch (op) {
      case Opcode::PUSH_INT: {
          auto val = prog_.Read<int64_t>(pc_);
          Push(val);
          continue;
      }

      case Opcode::PUSH_FUNC: {
        Push(prog_.Read<size_t>(pc_));
        continue;
      }
      case Opcode::PUSH_PROTO: {
        Push(prog_.Read<RuntimeFn>(pc_));
        continue;
      }
      case Opcode::PEEK: {
        auto idx = prog_.Read<unsigned>(pc_);
        Push(*(stack_.rbegin() + idx));
        continue;
      }
      case Opcode::POP: {
        Pop();
        continue;
      }
      case Opcode::CALL: {
        auto callee = Pop();
        switch (callee.Kind) {
          case Value::Kind::PROTO: {
            (*callee.Val.Proto) (*this);
            continue;
          }
          case Value::Kind::ADDR: {
            Push(pc_);
            pc_ = callee.Val.Addr;
            continue;
          }
          case Value::Kind::INT: {
            throw RuntimeError("cannot call integer");
          }
        }
        continue;
      }

      case Opcode::ADD: {
        auto rhs = PopInt();
        auto lhs = PopInt();

        uint64_t sum = (uint64_t)rhs + (uint64_t)lhs;
        
        bool b_rhs = rhs >> 63;
        bool b_lhs = lhs >> 63;
        bool b_sum = sum >> 63;
        
        if(!b_rhs && !b_lhs && b_sum == 1){
          throw RuntimeError("ADD operation overflow for positive values.");
        }

        if(b_rhs && b_lhs && b_sum == 0){
          throw RuntimeError("ADD operation overflow for negative values.");
        }
        Push(lhs + rhs);
        continue;
      }


      case Opcode::MUL:{
        auto rhs = PopInt();
        auto lhs = PopInt();

        int64_t multiply = (uint64_t)rhs * (uint64_t)lhs;
        
        bool b_rhs = rhs >> 63;
        bool b_lhs = lhs >> 63;
        bool b_multiply = multiply >> 63;
        
        // if(!b_rhs && !b_lhs && b_multiply == 1){
        //   throw RuntimeError("ADD operation overflow for positive values.");
        // }

        // if(b_rhs && b_lhs && b_multiply == 0){
        //   throw RuntimeError("ADD operation overflow for negative values.");
        // }
        Push(multiply);
        continue;
      }


  case Opcode::EQ: {
        auto rhs = PopInt();
        auto lhs = PopInt();

        int64_t rez = (rhs==lhs) ? 1 : 0;
        Push(rez);
        continue;
      }


      case Opcode::RET: {
        auto depth = prog_.Read<unsigned>(pc_);
        auto nargs = prog_.Read<unsigned>(pc_);
        auto v = Pop();
        stack_.resize(stack_.size() - depth);
        pc_ = PopAddr();
        stack_.resize(stack_.size() - nargs);
        Push(v);
        continue;
      }
      case Opcode::JUMP_FALSE: {
        auto cond = Pop();
        auto addr = prog_.Read<size_t>(pc_);
        if (!cond) {
          pc_ = addr;
        }
        continue;
      }
      case Opcode::JUMP: {
        pc_ = prog_.Read<size_t>(pc_);
        continue;
      }
      case Opcode::STOP: {
        return;
      }
    }
  }
}