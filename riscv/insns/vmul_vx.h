// vmul vd, vs2, rs1
VI_VX_LOOP
({
  vd = vs2 * rs1;
})
P.get_state()->mhpmcounter[10]->bump(1);
