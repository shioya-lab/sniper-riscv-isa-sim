// vsll
VI_VV_LOOP
({
  vd = vs2 << (vs1 & (sew - 1));
})
P.get_state()->mhpmcounter[10]->bump(1);
