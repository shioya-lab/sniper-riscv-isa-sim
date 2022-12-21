// vrsub.vi vd, vs2, imm, vm   # vd[i] = imm - vs2[i]
VI_VI_LOOP
({
  vd = simm5 - vs2;
})
P.get_state()->mhpmcounter[10]->bump(1);
