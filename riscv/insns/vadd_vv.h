// vadd.vv vd, vs1, vs2, vm
VI_VV_LOOP
({
  vd = vs1 + vs2;
})
P.get_state()->mhpmcounter[10]->bump(1);
