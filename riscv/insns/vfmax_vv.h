// vfmax
VI_VFP_VV_LOOP
({
  vd = f16_max(vs2, vs1);
},
{
  vd = f32_max(vs2, vs1);
},
{
  vd = f64_max(vs2, vs1);
})
P.get_state()->mhpmcounter[10]->bump(1);
