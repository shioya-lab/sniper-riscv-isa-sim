if(RS1 >= RS2) {
  set_pc(BRANCH_TARGET);
  LOG_BRANCH(true);
} else {
  LOG_BRANCH(false);
}
