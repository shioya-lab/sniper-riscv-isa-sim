// vsltu.vx  vd, vs2, vs1
VI_VX_ULOOP_CMP
({
  res = vs2 < rs1;
})
P.get_state()->mhpmcounter[10]->bump(1);
