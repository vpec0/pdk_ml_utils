#!/bin/bash

FNAME="vdgeomdev/srcs/dunecore/dunecore/Geometry/gdml/dunevd10kt_3view_v4_refactored_new_full_nowires.gdml"
[[ $# > 0 ]] && FNAME=$1

{ cat <<EOF
  {
      TGeoManager::Import("$FNAME");

      gGeoManager->GetVolume("volCryostat")->Draw("ogl") ;

      std::vector<TString> toHide = {
//	  "volSteelShell",
//	  "volExternalActive",
//	  "volGaseousArgon"
      };

      for(auto s: toHide) {
	     cout<<"Hiding "<<s<<endl;
	     gGeoManager->GetVolume(s)->SetVisibility(0);
      }

      auto vw = (TGLViewer*)gPad->GetViewer3D();
      vw->SetCurrentCamera(TGLViewer::kCameraPerspYOZ);
      // vw->CurrentCamera().RotateRad(0.5, 0.2);
  }
EOF
} > temp_view.C

root -l temp_view.C

rm temp_view.C
