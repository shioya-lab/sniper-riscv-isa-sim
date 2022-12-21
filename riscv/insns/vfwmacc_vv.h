// vfwmacc.vv vd, vs2, vs1
VI_VFP_VV_LOOP_WIDE
({
  vd = f32_mulAdd(vs1, vs2, vd);
},
{
  vd = f64_mulAdd(vs1, vs2, vd);
})
P.get_state()->mhpmcounter[10]->bump(1);
