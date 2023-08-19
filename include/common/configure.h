// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2019-2022 Second State INC

//===-- wasmedge/common/configure.h - Configuration class -----------------===//
//
// Part of the WasmEdge Project.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contents the configuration class of proposals, pre-registration
/// host functions, etc.
///
//===----------------------------------------------------------------------===//
#pragma once

#include "common/enum_configure.hpp"
#include "errinfo.h"

#include <atomic>
#include <bitset>
#include <cstdint>
#include <initializer_list>
#include <mutex>
#include <shared_mutex>
#include <unordered_set>

namespace WasmEdge {

class CompilerConfigure {
public:
  CompilerConfigure() noexcept = default;
  CompilerConfigure(const CompilerConfigure &RHS) noexcept
      : OptLevel(RHS.OptLevel.load(std::memory_order_relaxed)),
        OFormat(RHS.OFormat.load(std::memory_order_relaxed)),
        DumpIR(RHS.DumpIR.load(std::memory_order_relaxed)),
        GenericBinary(RHS.GenericBinary.load(std::memory_order_relaxed)),
        Interruptible(RHS.Interruptible.load(std::memory_order_relaxed)) {}

  /// AOT compiler optimization level enum class.
  enum class OptimizationLevel : uint8_t {
    // Disable as many optimizations as possible.
    O0,
    // Optimize quickly without destroying debuggability.
    O1,
    // Optimize for fast execution as much as possible without triggering
    // significant incremental compile time or code size growth.
    O2,
    // Optimize for fast execution as much as possible.
    O3,
    // Optimize for small code size as much as possible without triggering
    // significant incremental compile time or execution time slowdowns.
    Os,
    // Optimize for small code size as much as possible.
    Oz
  };
  void setOptimizationLevel(OptimizationLevel Level) noexcept {
    OptLevel.store(Level, std::memory_order_relaxed);
  }
  OptimizationLevel getOptimizationLevel() const noexcept {
    return OptLevel.load(std::memory_order_relaxed);
  }

  /// AOT compiler output binary format.
  enum class OutputFormat : uint8_t {
    // Native dynamic library format.
    Native,
    // WebAssembly with AOT compiled codes in custom sections.
    Wasm,
  };
  void setOutputFormat(OutputFormat Format) noexcept {
    OFormat.store(Format, std::memory_order_relaxed);
  }
  OutputFormat getOutputFormat() const noexcept {
    return OFormat.load(std::memory_order_relaxed);
  }

  void setDumpIR(bool IsDump) noexcept {
    DumpIR.store(IsDump, std::memory_order_relaxed);
  }

  bool isDumpIR() const noexcept {
    return DumpIR.load(std::memory_order_relaxed);
  }

  void setGenericBinary(bool IsGenericBinary) noexcept {
    GenericBinary.store(IsGenericBinary, std::memory_order_relaxed);
  }

  bool isGenericBinary() const noexcept {
    return GenericBinary.load(std::memory_order_relaxed);
  }

  void setInterruptible(bool IsInterruptible) noexcept {
    Interruptible.store(IsInterruptible, std::memory_order_relaxed);
  }

  bool isInterruptible() const noexcept {
    return Interruptible.load(std::memory_order_relaxed);
  }

private:
  std::atomic<OptimizationLevel> OptLevel = OptimizationLevel::O3;
  std::atomic<OutputFormat> OFormat = OutputFormat::Wasm;
  std::atomic<bool> DumpIR = false;
  std::atomic<bool> GenericBinary = false;
  std::atomic<bool> Interruptible = false;
};

class RuntimeConfigure {
public:
  RuntimeConfigure() noexcept = default;
  RuntimeConfigure(const RuntimeConfigure &RHS) noexcept
      : MaxMemPage(RHS.MaxMemPage.load(std::memory_order_relaxed)),
        ForceInterpreter(RHS.ForceInterpreter.load(std::memory_order_relaxed)) {
  }

  void setMaxMemoryPage(const uint32_t Page) noexcept {
    MaxMemPage.store(Page, std::memory_order_relaxed);
  }

  uint32_t getMaxMemoryPage() const noexcept {
    return MaxMemPage.load(std::memory_order_relaxed);
  }

  void setForceInterpreter(bool IsForceInterpreter) noexcept {
    ForceInterpreter.store(IsForceInterpreter, std::memory_order_relaxed);
  }

  bool isForceInterpreter() const noexcept {
    return ForceInterpreter.load(std::memory_order_relaxed);
  }

private:
  std::atomic<uint32_t> MaxMemPage = 65536;
  std::atomic<bool> ForceInterpreter = false;
};

class StatisticsConfigure {
public:
  StatisticsConfigure() noexcept = default;
  StatisticsConfigure(const StatisticsConfigure &RHS) noexcept
      : InstrCounting(RHS.InstrCounting.load(std::memory_order_relaxed)),
        CostMeasuring(RHS.CostMeasuring.load(std::memory_order_relaxed)),
        TimeMeasuring(RHS.TimeMeasuring.load(std::memory_order_relaxed)) {}

  void setInstructionCounting(bool IsCount) noexcept {
    InstrCounting.store(IsCount, std::memory_order_relaxed);
  }

  bool isInstructionCounting() const noexcept {
    return InstrCounting.load(std::memory_order_relaxed);
  }

  void setCostMeasuring(bool IsMeasure) noexcept {
    CostMeasuring.store(IsMeasure, std::memory_order_relaxed);
  }

  bool isCostMeasuring() const noexcept {
    return CostMeasuring.load(std::memory_order_relaxed);
  }

  void setTimeMeasuring(bool IsTimeMeasure) noexcept {
    TimeMeasuring.store(IsTimeMeasure, std::memory_order_relaxed);
  }

  bool isTimeMeasuring() const noexcept {
    return TimeMeasuring.load(std::memory_order_relaxed);
  }

  void setCostLimit(uint64_t Cost) noexcept {
    CostLimit.store(Cost, std::memory_order_relaxed);
  }

  uint64_t getCostLimit() const noexcept {
    return CostLimit.load(std::memory_order_relaxed);
  }

private:
  std::atomic<bool> InstrCounting = false;
  std::atomic<bool> CostMeasuring = false;
  std::atomic<bool> TimeMeasuring = false;
  std::atomic<uint64_t> CostLimit = UINT64_C(-1);
};

class Configure {
public:
  Configure() noexcept {
    unsafeAddProposal(Proposal::ImportExportMutGlobals);
    unsafeAddProposal(Proposal::NonTrapFloatToIntConversions);
    unsafeAddProposal(Proposal::SignExtensionOperators);
    unsafeAddProposal(Proposal::MultiValue);
    unsafeAddProposal(Proposal::BulkMemoryOperations);
    unsafeAddProposal(Proposal::ReferenceTypes);
    unsafeAddProposal(Proposal::SIMD);
  }
  template <typename... ArgsT> Configure(ArgsT... Args) noexcept : Configure() {
    (unsafeAddSet(Args), ...);
  }
  Configure(const Configure &RHS) noexcept
      : Proposals(RHS.Proposals), Hosts(RHS.Hosts),
        ForbiddenPlugins(RHS.ForbiddenPlugins), CompilerConf(RHS.CompilerConf),
        RuntimeConf(RHS.RuntimeConf), StatisticsConf(RHS.StatisticsConf) {}

  void addProposal(const Proposal Type) noexcept {
    std::unique_lock Lock(Mutex);
    unsafeAddProposal(Type);
  }

  void removeProposal(const Proposal Type) noexcept {
    std::unique_lock Lock(Mutex);
    Proposals.reset(static_cast<uint8_t>(Type));
  }

  bool hasProposal(const Proposal Type) const noexcept {
    std::shared_lock Lock(Mutex);
    return Proposals.test(static_cast<uint8_t>(Type));
  }

  void addHostRegistration(const HostRegistration Host) noexcept {
    std::unique_lock Lock(Mutex);
    Hosts.set(static_cast<uint8_t>(Host));
  }

  void removeHostRegistration(const HostRegistration Host) noexcept {
    std::unique_lock Lock(Mutex);
    Hosts.reset(static_cast<uint8_t>(Host));
  }

  void addForbiddenPlugins(std::string PluginName) noexcept {
    std::unique_lock Lock(Mutex);
    ForbiddenPlugins.emplace(std::move(PluginName));
  }

  bool isForbiddenPlugins(std::string PluginName) const noexcept {
    std::shared_lock Lock(Mutex);
    return ForbiddenPlugins.find(PluginName) != ForbiddenPlugins.end();
  }

  bool hasHostRegistration(const HostRegistration Host) const noexcept {
    std::shared_lock Lock(Mutex);
    return Hosts.test(static_cast<uint8_t>(Host));
  }

  const CompilerConfigure &getCompilerConfigure() const noexcept {
    return CompilerConf;
  }
  CompilerConfigure &getCompilerConfigure() noexcept { return CompilerConf; }

  const RuntimeConfigure &getRuntimeConfigure() const noexcept {
    return RuntimeConf;
  }
  RuntimeConfigure &getRuntimeConfigure() noexcept { return RuntimeConf; }

  const StatisticsConfigure &getStatisticsConfigure() const noexcept {
    return StatisticsConf;
  }
  StatisticsConfigure &getStatisticsConfigure() noexcept {
    return StatisticsConf;
  }

  inline auto logCheckError(ErrCode Code, ASTNodeAttr Node) const noexcept {
    spdlog::error(Code);
    spdlog::error(ErrInfo::InfoAST(Node));
    return Unexpect(Code);
  }
  inline auto logNeedProposal(ErrCode Code, Proposal Prop,
                              ASTNodeAttr Node) const noexcept {
    spdlog::error(Code);
    spdlog::error(ErrInfo::InfoProposal(Prop));
    spdlog::error(ErrInfo::InfoAST(Node));
    return Unexpect(Code);
  }

  // Helper function of checking the valid value types.
  Expect<void> checkValTypeProposals(ValType VType,
                                     ASTNodeAttr Node) const noexcept {
    switch (VType) {
    case ValType::I32:
    case ValType::I64:
    case ValType::F32:
    case ValType::F64:
      return {};
    case ValType::V128:
      if (!this->hasProposal(Proposal::SIMD)) {
        return logNeedProposal(ErrCode::Value::MalformedValType, Proposal::SIMD,
                               Node);
      }
      return {};
    case ValType::FuncRef:
      if (!this->hasProposal(Proposal::BulkMemoryOperations)) {
        return logNeedProposal(ErrCode::Value::MalformedElemType,
                               Proposal::BulkMemoryOperations, Node);
      }
      [[fallthrough]];
    case ValType::ExternRef:
      if (!this->hasProposal(Proposal::ReferenceTypes)) {
        return logNeedProposal(ErrCode::Value::MalformedElemType,
                               Proposal::ReferenceTypes, Node);
      }
      return {};
    default:
      return logCheckError(ErrCode::Value::MalformedValType, Node);
      ;
    }
  }

  // Helper function of checking the valid reference types.
  Expect<void> checkRefTypeProposals(RefType RType,
                                     ASTNodeAttr Node) const noexcept {
    switch (RType) {
    case RefType::ExternRef:
      if (!this->hasProposal(Proposal::ReferenceTypes)) {
        return logNeedProposal(ErrCode::Value::MalformedElemType,
                               Proposal::ReferenceTypes, Node);
      }
      [[fallthrough]];
    case RefType::FuncRef:
      return {};
    default:
      if (this->hasProposal(Proposal::ReferenceTypes)) {
        return logCheckError(ErrCode::Value::MalformedRefType, Node);
      } else {
        return logCheckError(ErrCode::Value::MalformedElemType, Node);
      }
    }
  }

  Expect<void> checkInstrProposals(OpCode Code) const noexcept {
    if (Code >= OpCode::I32__trunc_sat_f32_s &&
        Code <= OpCode::I64__trunc_sat_f64_u) {
      // These instructions are for NonTrapFloatToIntConversions proposal.
      if (unlikely(
              !this->hasProposal(Proposal::NonTrapFloatToIntConversions))) {
        return logNeedProposal(ErrCode::Value::IllegalOpCode,
                               Proposal::NonTrapFloatToIntConversions,
                               ASTNodeAttr::Instruction);
      }
    } else if (Code >= OpCode::I32__extend8_s &&
               Code <= OpCode::I64__extend32_s) {
      // These instructions are for SignExtensionOperators proposal.
      if (unlikely(!this->hasProposal(Proposal::SignExtensionOperators))) {
        return logNeedProposal(ErrCode::Value::IllegalOpCode,
                               Proposal::SignExtensionOperators,
                               ASTNodeAttr::Instruction);
      }
    } else if ((Code >= OpCode::Ref__null && Code <= OpCode::Ref__func) ||
               (Code >= OpCode::Table__init && Code <= OpCode::Table__copy) ||
               (Code >= OpCode::Memory__init && Code <= OpCode::Memory__fill)) {
      // These instructions are for ReferenceTypes or BulkMemoryOperations
      // proposal.
      if (unlikely(!this->hasProposal(Proposal::ReferenceTypes)) &&
          unlikely(!this->hasProposal(Proposal::BulkMemoryOperations))) {
        return logNeedProposal(ErrCode::Value::IllegalOpCode,
                               Proposal::ReferenceTypes,
                               ASTNodeAttr::Instruction);
      }
    } else if (Code == OpCode::Select_t ||
               (Code >= OpCode::Table__get && Code <= OpCode::Table__set) ||
               (Code >= OpCode::Table__grow && Code <= OpCode::Table__fill)) {
      // These instructions are for ReferenceTypes proposal.
      if (unlikely(!this->hasProposal(Proposal::ReferenceTypes))) {
        return logNeedProposal(ErrCode::Value::IllegalOpCode,
                               Proposal::ReferenceTypes,
                               ASTNodeAttr::Instruction);
      }
    } else if (Code >= OpCode::V128__load &&
               Code <= OpCode::F64x2__convert_low_i32x4_u) {
      // These instructions are for SIMD proposal.
      if (!this->hasProposal(Proposal::SIMD)) {
        return logNeedProposal(ErrCode::Value::IllegalOpCode, Proposal::SIMD,
                               ASTNodeAttr::Instruction);
      }
    } else if (Code == OpCode::Return_call ||
               Code == OpCode::Return_call_indirect) {
      // These instructions are for TailCall proposal.
      if (!this->hasProposal(Proposal::TailCall)) {
        return logNeedProposal(ErrCode::Value::IllegalOpCode,
                               Proposal::TailCall, ASTNodeAttr::Instruction);
      }
    } else if (Code >= OpCode::I32__atomic__load &&
               Code <= OpCode::I64__atomic__rmw32__cmpxchg_u) {
      // These instructions are for Thread proposal.
      if (!this->hasProposal(Proposal::Threads)) {
        return logNeedProposal(ErrCode::Value::IllegalOpCode, Proposal::Threads,
                               ASTNodeAttr::Instruction);
      }
    }
    return {};
  }

private:
  void unsafeAddSet(const Proposal P) noexcept { unsafeAddProposal(P); }
  void unsafeAddSet(const HostRegistration H) noexcept {
    unsafeAddHostRegistration(H);
  }

  void unsafeAddProposal(const Proposal Type) noexcept {
    Proposals.set(static_cast<uint8_t>(Type));
  }

  void unsafeAddHostRegistration(const HostRegistration Host) noexcept {
    Hosts.set(static_cast<uint8_t>(Host));
  }

  mutable std::shared_mutex Mutex;
  std::bitset<static_cast<uint8_t>(Proposal::Max)> Proposals;
  std::bitset<static_cast<uint8_t>(HostRegistration::Max)> Hosts;
  std::unordered_set<std::string> ForbiddenPlugins;

  CompilerConfigure CompilerConf;
  RuntimeConfigure RuntimeConf;
  StatisticsConfigure StatisticsConf;
};

} // namespace WasmEdge
