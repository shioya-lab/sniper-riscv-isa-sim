// vminu.vv vd, vs2, vs1, vm   # Vector-vector
VI_VV_ULOOP
({
  if (vs1 <= vs2) {
    vd = vs1;
  } else {
    vd = vs2;
  }
})
P.get_state()->mhpmcounter[10]->bump(1);
