// vsltu.vv  vd, vs2, vs1
VI_VV_ULOOP_CMP
({
  res = vs2 < vs1;
})
P.get_state()->mhpmcounter[10]->bump(1);
