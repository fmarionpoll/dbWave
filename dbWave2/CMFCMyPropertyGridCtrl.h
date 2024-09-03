#pragma once
#include <afxpropertygridctrl.h>

#include "ColorNames.h"

class CMFCMyPropertyGridCtrl :
    public CMFCPropertyGridCtrl
{
public:

    virtual int OnDrawProperty(CDC* p_dc, CMFCPropertyGridProperty* p_prop) const {
        // Implement check to see, if this is the property we are looking for
        // If it is, set an appropriate text color
        int class_index = p_prop->GetData();
        COLORREF color = color_spike_class[class_index];
        p_dc->SetTextColor(color);

        // Call the default implementation to perform rendering
        return CMFCPropertyGridCtrl::OnDrawProperty(p_dc, p_prop);
    }

    COLORREF color_spike_class[10] =
    {
        col_black,
        RGB(126, 132, 250), // 6
        RGB(246, 133, 17), // 3
        RGB(15, 181, 174), // 1
        RGB(197, 198, 198), //4
        RGB(114, 224, 106), // 7
        RGB(222, 61, 130), // 5
        RGB(20, 122, 243), // 8
        RGB(64, 70, 202), // 2
        RGB(115, 38, 211), // 9
    };

};

