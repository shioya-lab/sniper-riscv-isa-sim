// vredxor.vs vd, vs2 ,vs1
VI_VV_LOOP_REDUCTION
({
  vd_0_res ^= vs2;
})
P.get_state()->mhpmcounter[10]->bump(1);
