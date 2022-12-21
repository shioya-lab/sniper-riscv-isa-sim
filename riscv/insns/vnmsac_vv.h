// vmsac.vv: vd[i] = -(vs1[i] * vs2[i]) + vd[i]
VI_VV_LOOP
({
  vd = -(vs1 * vs2) + vd;
})
P.get_state()->mhpmcounter[10]->bump(1);
