// vslt.vv  vd, vd2, vs1
VI_VV_LOOP_CMP
({
  res = vs2 < vs1;
})
P.get_state()->mhpmcounter[10]->bump(1);
