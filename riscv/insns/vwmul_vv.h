// vwmul.vv vd, vs2, vs1
VI_CHECK_DSS(true);
VI_VV_LOOP_WIDEN
({
  VI_WIDE_OP_AND_ASSIGN(vs2, vs1, 0, *, +, int);
})
P.get_state()->mhpmcounter[10]->bump(1);
