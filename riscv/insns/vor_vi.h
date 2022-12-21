// vor
VI_VI_LOOP
({
  vd = simm5 | vs2;
})
P.get_state()->mhpmcounter[10]->bump(1);
