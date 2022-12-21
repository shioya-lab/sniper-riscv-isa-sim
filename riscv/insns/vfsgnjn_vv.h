// vfsgnn
VI_VFP_VV_LOOP
({
  vd = fsgnj16(vs2.v, vs1.v, true, false);
},
{
  vd = fsgnj32(vs2.v, vs1.v, true, false);
},
{
  vd = fsgnj64(vs2.v, vs1.v, true, false);
})
P.get_state()->mhpmcounter[10]->bump(1);
