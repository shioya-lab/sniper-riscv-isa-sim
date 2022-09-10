if (sreg_t(RS1) < sreg_t(RS2)) {
  set_pc(BRANCH_TARGET);
  LOG_BRANCH(true);
} else {
  LOG_BRANCH(false);
}
