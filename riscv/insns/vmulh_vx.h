// vmulh vd, vs2, rs1
VI_VX_LOOP
({
  vd = ((int128_t)vs2 * rs1) >> sew;
})
P.get_state()->mhpmcounter[10]->bump(1);
