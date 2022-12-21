// vmulhsu.vx vd, vs2, rs1
VI_VX_SU_LOOP({
  vd = ((int128_t)vs2 * (uint128_t)rs1) >> sew;
})
P.get_state()->mhpmcounter[10]->bump(1);
