// vremu.vv vd, vs2, vs1
VI_VV_ULOOP
({
  if (vs1 == 0)
    vd = vs2;
  else
    vd = vs2 % vs1;
})
P.get_state()->mhpmcounter[10]->bump(1);
