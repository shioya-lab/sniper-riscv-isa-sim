// vsrl.vv  vd, vs2, vs1
VI_VV_ULOOP
({
  vd = vs2 >> (vs1 & (sew - 1));
})
P.get_state()->mhpmcounter[10]->bump(1);
