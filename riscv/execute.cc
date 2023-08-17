// See LICENSE for license details.

#include "processor.h"
#include "mmu.h"
#include "disasm.h"
#include <cassert>

#ifdef RISCV_ENABLE_SIFT
# include "sift_writer.h"
#endif

#ifdef RISCV_ENABLE_COMMITLOG
static void commit_log_reset(processor_t* p)
{
  p->get_state()->log_reg_write.clear();
  p->get_state()->log_mem_read.clear();
  p->get_state()->log_mem_write.clear();
  p->get_state()->log_addr_valid = 0;
  p->get_state()->log_is_branch = false;
  p->get_state()->log_is_branch_taken = false;
}

static void commit_log_stash_privilege(processor_t* p)
{
  state_t* state = p->get_state();
  state->last_inst_priv = state->prv;
  state->last_inst_xlen = p->get_xlen();
  state->last_inst_flen = p->get_flen();
}

uint32_t sift_executed_insn;


void record_executed_insn (uint64_t insn)
{
  sift_executed_insn = insn;
}

static void commit_log_print_value(FILE *log_file, int width, const void *data)
{
  assert(log_file);

  switch (width) {
    case 8:
      fprintf(log_file, "0x%01" PRIx8, *(const uint8_t *)data);
      break;
    case 16:
      fprintf(log_file, "0x%04" PRIx16, *(const uint16_t *)data);
      break;
    case 32:
      fprintf(log_file, "0x%08" PRIx32, *(const uint32_t *)data);
      break;
    case 64:
      fprintf(log_file, "0x%016" PRIx64, *(const uint64_t *)data);
      break;
    default:
      // max lengh of vector
      if (((width - 1) & width) == 0) {
        const uint64_t *arr = (const uint64_t *)data;

        fprintf(log_file, "0x");
        for (int idx = width / 64 - 1; idx >= 0; --idx) {
          fprintf(log_file, "%016" PRIx64, arr[idx]);
        }
      } else {
        abort();
      }
      break;
  }
}

static void commit_log_print_value(FILE *log_file, int width, uint64_t val)
{
  commit_log_print_value(log_file, width, &val);
}

const char* processor_t::get_symbol(uint64_t addr)
{
  return sim->get_symbol(addr);
}

static void commit_log_print_insn(processor_t *p, reg_t pc, insn_t insn)
{
  FILE *log_file = p->get_log_file();

  auto& reg = p->get_state()->log_reg_write;
  auto& load = p->get_state()->log_mem_read;
  auto& store = p->get_state()->log_mem_write;
  int priv = p->get_state()->last_inst_priv;
  int xlen = p->get_state()->last_inst_xlen;
  int flen = p->get_state()->last_inst_flen;

  // print core id on all lines so it is easy to grep
  fprintf(log_file, "core%4" PRId32 ": ", p->get_id());

  fprintf(log_file, "%1d ", priv);
  commit_log_print_value(log_file, xlen, pc);
  fprintf(log_file, " (");
  commit_log_print_value(log_file, insn.length() * 8, insn.bits());
  fprintf(log_file, ")");
  bool show_vec = false;

  for (auto item : reg) {
    if (item.first == 0)
      continue;

    char prefix;
    int size;
    int rd = item.first >> 4;
    bool is_vec = false;
    bool is_vreg = false;
    switch (item.first & 0xf) {
    case 0:
      size = xlen;
      prefix = 'x';
      break;
    case 1:
      size = flen;
      prefix = 'f';
      break;
    case 2:
      size = p->VU.VLEN;
      prefix = 'v';
      is_vreg = true;
      break;
    case 3:
      is_vec = true;
      break;
    case 4:
      size = xlen;
      prefix = 'c';
      break;
    default:
      assert("can't been here" && 0);
      break;
    }

    if (!show_vec && (is_vreg || is_vec)) {
        fprintf(log_file, " e%ld %s%ld l%ld",
                p->VU.vsew,
                p->VU.vflmul < 1 ? "mf" : "m",
                p->VU.vflmul < 1 ? (reg_t)(1 / p->VU.vflmul) : (reg_t)p->VU.vflmul,
                p->VU.vl->read());
        show_vec = true;
    }

    if (!is_vec) {
      if (prefix == 'c')
        fprintf(log_file, " c%d_%s ", rd, csr_name(rd));
      else
        fprintf(log_file, " %c%-2d ", prefix, rd);
      if (is_vreg)
        commit_log_print_value(log_file, size, &p->VU.elt<uint8_t>(rd, 0));
      else
        commit_log_print_value(log_file, size, item.second.v);
    }
  }

  for (auto item : load) {
    fprintf(log_file, " mem ");
    commit_log_print_value(log_file, xlen, std::get<0>(item));
  }

  for (auto item : store) {
    fprintf(log_file, " mem ");
    commit_log_print_value(log_file, xlen, std::get<0>(item));
    fprintf(log_file, " ");
    commit_log_print_value(log_file, std::get<2>(item) << 3, std::get<1>(item));
  }
  fprintf(log_file, "\n");
}
#else
static void commit_log_reset(processor_t* p) {}
static void commit_log_stash_privilege(processor_t* p) {}
static void commit_log_print_insn(processor_t* p, reg_t pc, insn_t insn) {}
#endif

inline void processor_t::update_histogram(reg_t pc)
{
#ifdef RISCV_ENABLE_HISTOGRAM
  pc_histogram[pc]++;
#endif
}



static void log_print_mem_access (processor_t *p, reg_t pc, insn_t insn)
{
  auto& load = p->get_state()->log_mem_read;
  auto& store = p->get_state()->log_mem_write;

  const reg_t VEC_ARITH_MATCH = 0x57;
  const reg_t VEC_LOAD_MATCH  = 0x07;
  const reg_t VEC_STORE_MATCH = 0x27;

  bool is_vector = false;

  auto insn_bit = insn.bits();
  if ((insn_bit & 0x7f) == VEC_ARITH_MATCH ||
      (insn_bit & 0x7f) == VEC_LOAD_MATCH  ||
      (insn_bit & 0x7f) == VEC_STORE_MATCH) {
    is_vector = true;
  }

  reg_t minstret = p->get_state()->minstret->read();
  for (auto item : store) {
    auto addr = std::get<0>(item);
    if (p->addr_history.find(addr) == p->addr_history.end()) {
      p->addr_history[addr] = new processor_t::period_info_t(new processor_t::period_t(minstret, is_vector));
    } else {
      p->addr_history[addr]->set_last_access (minstret);
    }
  }

  for (auto item : load) {
    auto addr = std::get<0>(item);
    if (p->addr_history.find(addr) == p->addr_history.end()) {
      p->addr_history[addr] = new processor_t::period_info_t(new processor_t::period_t(minstret, is_vector));
    } else {
      auto last_access = p->addr_history[addr]->get_last_access();
      if (addr == 0x0041ad20) {
        fprintf (stderr, "set 0x004ad20, last_access = %ld, current_access = %ld, period = %ld\n",
                 last_access, minstret, minstret - last_access);
      }
      p->addr_history[addr]->get_period_list()->push_back(new processor_t::period_t(minstret - last_access, is_vector));
      p->addr_history[addr]->set_last_access (minstret);
    }
  }
}

static void log_print_sift_trace(processor_t* p, reg_t pc, insn_t insn)
{
#ifdef RISCV_ENABLE_SIFT
  uint64_t addr = pc;
  uint64_t size = insn.length();
  uint64_t num_addresses = p->get_state()->log_addr_valid;
  uint64_t *addresses = p->get_state()->log_addr;
  reg_t    *wr_regs = p->get_state()->log_reg_addr;
  bool     is_branch = p->get_state()->log_is_branch;
  bool     taken = p->get_state()->log_is_branch_taken;

  auto& reg = p->get_state()->log_reg_write;

  std::vector<reg_t>vreg_array;
  for (auto i: reg) {
    if ((i.first & 0xf) == 2) {
      vreg_array.push_back(i.first >> 4);
    }
  }
  for (uint64_t addr_i = 0; addr_i < num_addresses; addr_i++) {
    if (std::find(vreg_array.begin(), vreg_array.end(), wr_regs[addr_i]) == vreg_array.end()) {
      vreg_array.push_back(wr_regs[addr_i]);
    }
  }

  std::sort(vreg_array.begin(), vreg_array.end() );

  if (vreg_array.size() > 0) {
    for (reg_t vreg_idx = 0; vreg_idx < vreg_array.size(); vreg_idx++) {
      uint64_t uop_addresses[1024];

      uint64_t num_mem_addr = 0;

      // fprintf (stderr, "vreg_array.size() = %d, vreg_idx = %d\n", vreg_array.size(), vreg_array[vreg_idx]);

      for (uint64_t addr_i = 0; addr_i < num_addresses; addr_i++) {
        if (vreg_array[vreg_idx] == wr_regs[addr_i]) {
          uop_addresses[num_mem_addr++] = addresses[addr_i];
          // fprintf (stderr, "vreg = %d, address set to %08lx\n", vreg_array[vreg_idx], addresses[addr_i]);
        }
      }
      p->get_state()->log_writer->Instruction(addr, size, num_mem_addr, uop_addresses, is_branch, taken, 0 /*is_predicate*/, 1 /*executed*/);

      // One increment vd
      char vd_origin = (sift_executed_insn >> 7) & 0x01f;
      vd_origin++;
      // fprintf(stderr, "Before sift_executed_insn = %08lx\n", sift_executed_insn);
      sift_executed_insn = (sift_executed_insn & ~(0x1f << 7)) | (vd_origin << 7);
      // fprintf(stderr, "After sift_executed_insn = %08lx\n", sift_executed_insn);

      bool is_opivx = ((sift_executed_insn & 0x7f) == 0x57) &
          (((sift_executed_insn >> 12) & 0x7) == 0x4);
      bool is_opfvf = ((sift_executed_insn & 0x7f) == 0x57) &
          (((sift_executed_insn >> 12) & 0x7) == 0x5);
      bool is_opmvx = ((sift_executed_insn & 0x7f) == 0x57) &
          (((sift_executed_insn >> 12) & 0x7) == 0x6);

      bool is_opfvv_vfunary0 = ((sift_executed_insn & 0x7f) == 0x57) &
          (((sift_executed_insn >> 12) & 0x7) == 0x1) &
          (((sift_executed_insn >> 26) & 0x3f) == 0x12);
      bool is_opfvv_vfunary1 = ((sift_executed_insn & 0x7f) == 0x57) &
          (((sift_executed_insn >> 12) & 0x7) == 0x1) &
          (((sift_executed_insn >> 26) & 0x3f) == 0x13);
      bool is_opmvv_vxunary1 = ((sift_executed_insn & 0x7f) == 0x57) &
          (((sift_executed_insn >> 12) & 0x7) == 0x2) &
          (((sift_executed_insn >> 26) & 0x3f) == 0x13);
      bool is_opmvv_vmunary1 = ((sift_executed_insn & 0x7f) == 0x57) &
          (((sift_executed_insn >> 12) & 0x7) == 0x2) &
          (((sift_executed_insn >> 26) & 0x3f) == 0x14);

      bool is_vlx_indexed = ((sift_executed_insn & 0x7f) == 0x07) & (((sift_executed_insn >> 26) & 0x1) == 1);
      bool is_vsx_indexed = ((sift_executed_insn & 0x7f) == 0x27) & (((sift_executed_insn >> 26) & 0x1) == 1);

      bool is_opivi = ((sift_executed_insn & 0x7f) == 0x57) &
          (((sift_executed_insn >> 12) & 0x7) == 0x3);

      // One increment vs1
      if ((num_addresses == 0) &  // not memory instruction
          !is_opfvv_vfunary0 & !is_opfvv_vfunary1 &
          !is_opmvv_vxunary1 & !is_opmvv_vmunary1 &
          !is_opivx & !is_opfvf & !is_opmvx & !is_opivi) {
        char vs1_origin = (sift_executed_insn >> 15) & 0x01f;
        vs1_origin++;
        sift_executed_insn = (sift_executed_insn & ~(0x1f << 15)) | (vs1_origin << 15);
      }

      if (num_addresses == 0 ||  // arithmetic instruction
          is_vlx_indexed ||
          is_vsx_indexed) {
        if (is_opfvv_vfunary0 & (sift_executed_insn >> 18) & 1 || // Widening Convertion
            (MASK_VMV_V_X & sift_executed_insn) == MATCH_VMV_V_X) {  // VMV_V_X
          // Do nothing
        } else {
          // One increment vs2
          char vs2_origin = (sift_executed_insn >> 20) & 0x01f;
          vs2_origin++;
          sift_executed_insn = (sift_executed_insn & ~(0x1f << 20)) | (vs2_origin << 20);
        }
      }
    }
  } else {
    p->get_state()->log_writer->Instruction(addr, size, num_addresses, addresses, is_branch, taken, 0 /*is_predicate*/, 1 /*executed*/);
    if (sift_executed_insn == 0x00100013) {
      p->get_state()->log_writer->Magic (1, 0, 0);   // SIM_ROI_START = 1 at sim_api.h
    }
    if (sift_executed_insn == 0x00200013) {
      p->get_state()->log_writer->Magic (2, 0, 0);   // SIM_ROI_END = 2 at sim_api.h
    }
    if ((sift_executed_insn & MASK_VSETVLI) == MATCH_VSETVLI ||
        (sift_executed_insn & MASK_VSETIVLI) == MATCH_VSETIVLI ||
        (sift_executed_insn & MASK_VSETVL)   == MATCH_VSETVL) {
      auto vl = p->VU.vl;
      auto vtype = p->VU.vtype;
      auto vl_value = vl->read();
      auto vtype_value = vtype->read();
      p->get_state()->log_writer->Magic(5, vl_value, vtype_value);  // SIM_CMD_USER = 5 at sim_api.h
    }
  }

  // p->get_state()->log_addr = 0;
  p->get_state()->log_addr_valid = 0;
  p->get_state()->log_is_branch = false;
  p->get_state()->log_is_branch_taken = false;
#endif
}

// This is expected to be inlined by the compiler so each use of execute_insn
// includes a duplicated body of the function to get separate fetch.func
// function calls.
static inline reg_t execute_insn(processor_t* p, reg_t pc, insn_fetch_t fetch)
{
#ifdef RISCV_ENABLE_COMMITLOG
  commit_log_reset(p);
  commit_log_stash_privilege(p);
#endif

  reg_t npc;

  try {
    npc = fetch.func(p, fetch.insn, pc);
    if (npc != PC_SERIALIZE_BEFORE) {

#ifdef RISCV_ENABLE_COMMITLOG
      record_executed_insn (fetch.insn.bits());
      if (p->get_log_commits_enabled()) {
        commit_log_print_insn(p, pc, fetch.insn);
      }
#endif
      
      log_print_mem_access(p, pc, fetch.insn);
      log_print_sift_trace(p, pc, fetch.insn);

    }
#ifdef RISCV_ENABLE_COMMITLOG
  } catch (wait_for_interrupt_t &t) {
      record_executed_insn (fetch.insn.bits());
      if (p->get_log_commits_enabled()) {
        commit_log_print_insn(p, pc, fetch.insn);
        log_print_sift_trace(p, pc, fetch.insn);
      }
      throw;
  } catch(mem_trap_t& t) {
      //handle segfault in midlle of vector load/store
      if (p->get_log_commits_enabled()) {
        for (auto item : p->get_state()->log_reg_write) {
          if ((item.first & 3) == 3) {
            record_executed_insn (fetch.insn.bits());
            commit_log_print_insn(p, pc, fetch.insn);
            log_print_sift_trace(p, pc, fetch.insn);
            break;
          }
        }
      }
      throw;
#endif
  } catch(...) {
    throw;
  }
  p->update_histogram(pc);

  return npc;
}

bool processor_t::slow_path()
{
  return debug || state.single_step != state.STEP_NONE || state.debug_mode;
}

// fetch/decode/execute loop
void processor_t::step(size_t n)
{
  if (!state.debug_mode) {
    if (halt_request == HR_REGULAR) {
      enter_debug_mode(DCSR_CAUSE_DEBUGINT);
    } else if (halt_request == HR_GROUP) {
      enter_debug_mode(DCSR_CAUSE_GROUP);
    } // !!!The halt bit in DCSR is deprecated.
    else if (state.dcsr->halt) {
      enter_debug_mode(DCSR_CAUSE_HALT);
    }
  }

  while (n > 0) {
    size_t instret = 0;
    reg_t pc = state.pc;
    mmu_t* _mmu = mmu;

    #define advance_pc() \
      if (unlikely(invalid_pc(pc))) { \
        switch (pc) { \
          case PC_SERIALIZE_BEFORE: state.serialized = true; break; \
          case PC_SERIALIZE_AFTER: ++instret; break; \
          default: abort(); \
        } \
        pc = state.pc; \
        break; \
      } else { \
        state.pc = pc; \
        instret++; \
      }

    try
    {
      take_pending_interrupt();

      if (unlikely(slow_path()))
      {
        // Main simulation loop, slow path.
        while (instret < n)
        {
          if (unlikely(!state.serialized && state.single_step == state.STEP_STEPPED)) {
            state.single_step = state.STEP_NONE;
            if (!state.debug_mode) {
              enter_debug_mode(DCSR_CAUSE_STEP);
              // enter_debug_mode changed state.pc, so we can't just continue.
              break;
            }
          }

          if (unlikely(state.single_step == state.STEP_STEPPING)) {
            state.single_step = state.STEP_STEPPED;
          }

          insn_fetch_t fetch = mmu->load_insn(pc);
          if (debug && !state.serialized)
            disasm(fetch.insn);
          pc = execute_insn(this, pc, fetch);
          advance_pc();
        }
      }
      else while (instret < n)
      {
        // Main simulation loop, fast path.
        for (auto ic_entry = _mmu->access_icache(pc); ; ) {
          auto fetch = ic_entry->data;
          pc = execute_insn(this, pc, fetch);
          ic_entry = ic_entry->next;
          if (unlikely(ic_entry->tag != pc))
            break;
          if (unlikely(instret + 1 == n))
            break;
          instret++;
          state.pc = pc;
        }

        advance_pc();
      }
    }
    catch(trap_t& t)
    {
      take_trap(t, pc);
      n = instret;

      if (unlikely(state.single_step == state.STEP_STEPPED)) {
        state.single_step = state.STEP_NONE;
        enter_debug_mode(DCSR_CAUSE_STEP);
      }
    }
    catch (triggers::matched_t& t)
    {
      if (mmu->matched_trigger) {
        // This exception came from the MMU. That means the instruction hasn't
        // fully executed yet. We start it again, but this time it won't throw
        // an exception because matched_trigger is already set. (All memory
        // instructions are idempotent so restarting is safe.)

        insn_fetch_t fetch = mmu->load_insn(pc);
        pc = execute_insn(this, pc, fetch);
        advance_pc();

        delete mmu->matched_trigger;
        mmu->matched_trigger = NULL;
      }
      switch (t.action) {
        case triggers::ACTION_DEBUG_MODE:
          enter_debug_mode(DCSR_CAUSE_HWBP);
          break;
        case triggers::ACTION_DEBUG_EXCEPTION: {
          trap_breakpoint trap(state.v, t.address);
          take_trap(trap, pc);
          break;
        }
        default:
          abort();
      }
    }
    catch(trap_debug_mode&)
    {
      enter_debug_mode(DCSR_CAUSE_SWBP);
    }
    catch (wait_for_interrupt_t &t)
    {
      // Return to the outer simulation loop, which gives other devices/harts a
      // chance to generate interrupts.
      //
      // In the debug ROM this prevents us from wasting time looping, but also
      // allows us to switch to other threads only once per idle loop in case
      // there is activity.
      n = ++instret;
    }

    state.minstret->bump(instret);

    // Model a hart whose CPI is 1.
    state.mcycle->bump(instret);

    n -= instret;
  }
}
