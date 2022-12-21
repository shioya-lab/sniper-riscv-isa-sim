// vrsub.vx vd, vs2, rs1, vm   # vd[i] = rs1 - vs2[i]
VI_VX_LOOP
({
  vd = rs1 - vs2;
})
P.get_state()->mhpmcounter[10]->bump(1);
