require_extension('C');
if (RVC_RS1S == 0) {
  set_pc(pc + insn.rvc_b_imm());
  LOG_BRANCH(true);
} else {
  LOG_BRANCH(false);
}
