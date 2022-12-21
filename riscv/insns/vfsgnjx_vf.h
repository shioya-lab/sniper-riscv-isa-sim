// vfsgnx
VI_VFP_VF_LOOP
({
  vd = fsgnj16(vs2.v, rs1.v, false, true);
},
{
  vd = fsgnj32(vs2.v, rs1.v, false, true);
},
{
  vd = fsgnj64(vs2.v, rs1.v, false, true);
})
P.get_state()->mhpmcounter[10]->bump(1);
