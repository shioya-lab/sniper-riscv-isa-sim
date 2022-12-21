// vfsgnn
VI_VFP_VF_LOOP
({
  vd = fsgnj16(vs2.v, rs1.v, true, false);
},
{
  vd = fsgnj32(vs2.v, rs1.v, true, false);
},
{
  vd = fsgnj64(vs2.v, rs1.v, true, false);
})
P.get_state()->mhpmcounter[10]->bump(1);
