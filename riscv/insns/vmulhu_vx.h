// vmulhu vd ,vs2, rs1
VI_VX_ULOOP
({
  vd = ((uint128_t)vs2 * rs1) >> sew;
})
P.get_state()->mhpmcounter[10]->bump(1);
