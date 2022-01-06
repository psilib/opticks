#include <cstring>
#include "G4SystemOfUnits.hh"
#include "G4Polycone.hh"
#include "G4Sphere.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Cons.hh"
#include "G4SubtractionSolid.hh"
#include "G4ThreeVector.hh"
#include "G4Orb.hh"
#include "G4UnionSolid.hh"
#include "G4Ellipsoid.hh"

using CLHEP::pi ;

#include "SSys.hh"
#include "X4SolidMaker.hh"
#include "PLOG.hh"

const plog::Severity X4SolidMaker::LEVEL = PLOG::EnvLevel("X4SolidMaker", "DEBUG"); 

bool X4SolidMaker::StartsWith( const char* n, const char* q ) // static
{
    return strlen(q) >= strlen(n) && strncmp(q, n, strlen(n)) == 0 ; 
}

bool X4SolidMaker::CanMake(const char* qname) // static 
{
    bool found = false ; 
    std::stringstream ss(NAMES) ;    
    std::string name ; 
    while (!found && std::getline(ss, name)) if(!name.empty() && StartsWith(name.c_str(), qname)) found = true ;
    LOG(LEVEL) << " qname " << qname << " found " << found ; 
    return found ; 
}

const char* X4SolidMaker::NAMES = R"LITERAL(
Orb
SphereWithPhiSegment
SphereWithThetaSegment
AdditionAcrylicConstruction
XJfixtureConstruction
XJanchorConstruction
SJReceiverConstruction
BoxMinusTubs0
BoxMinusTubs1
BoxMinusOrb
UnionOfHemiEllipsoids
PolyconeWithMultipleRmin
)LITERAL"; 

const G4VSolid* PolyconeWithMultipleRmin(const char* name);

const G4VSolid* X4SolidMaker::Make(const char* qname)  // static
{
    const G4VSolid* solid = nullptr ; 
    if(     StartsWith("Orb",qname))                          solid = X4SolidMaker::Orb(qname); 
    else if(StartsWith("SphereWithPhiSegment",qname))         solid = X4SolidMaker::SphereWithPhiSegment(qname); 
    else if(StartsWith("SphereWithThetaSegment",qname))       solid = X4SolidMaker::SphereWithThetaSegment(qname); 
    else if(StartsWith("AdditionAcrylicConstruction",qname))  solid = X4SolidMaker::AdditionAcrylicConstruction(qname); 
    else if(StartsWith("XJfixtureConstruction", qname))       solid = X4SolidMaker::XJfixtureConstruction(qname); 
    else if(StartsWith("XJanchorConstruction", qname))        solid = X4SolidMaker::XJanchorConstruction(qname) ; 
    else if(StartsWith("SJReceiverConstruction", qname))      solid = X4SolidMaker::SJReceiverConstruction(qname) ; 
    else if(StartsWith("BoxMinusTubs0",qname))                solid = X4SolidMaker::BoxMinusTubs0(qname); 
    else if(StartsWith("BoxMinusTubs1",qname))                solid = X4SolidMaker::BoxMinusTubs1(qname); 
    else if(StartsWith("BoxMinusOrb",qname))                  solid = X4SolidMaker::BoxMinusOrb(qname); 
    else if(StartsWith("UnionOfHemiEllipsoids", qname))       solid = X4SolidMaker::UnionOfHemiEllipsoids(qname); 
    else if(StartsWith("PolyconeWithMultipleRmin", qname))    solid = X4SolidMaker::PolyconeWithMultipleRmin(qname) ; 

    assert(solid); 
    return solid ; 
}

const G4VSolid* X4SolidMaker::Orb(const char* name)  // static
{
    return new G4Orb(name, 100.) ; 
}

/**
X4SolidMaker::SphereWithPhiSegment
--------------------------------------

Best way to view phi segment is with XY cross section 

phi_start:0 phi_delta:2 
    full sphere in phi 

phi_start:0 phi_delta:0.5 
    cheese shape : suspect position of the cheese 
    may differ between Opticks and Geant4


**/

const G4VSolid* X4SolidMaker::SphereWithPhiSegment(const char* name)  // static
{
    double phi_start = SSys::getenvfloat("X4SolidMaker_SphereWithPhiSegment_phi_start", 0.f) ;  // units of pi
    double phi_delta = SSys::getenvfloat("X4SolidMaker_SphereWithPhiSegment_phi_delta", 0.5f) ; // units of pi

    LOG(info)
        << " (inputs are scaled by pi) "
        << " phi_start  " << phi_start
        << " phi_delta  " << phi_delta
        << " phi_start+phi_delta  " << phi_start+phi_delta
        << " phi_start*pi " << phi_start*pi
        << " phi_delta*pi " << phi_delta*pi
        << " (phi_start+phi_delta)*pi " << (phi_start+phi_delta)*pi
        ;

    G4String pName = name ; 
    G4double pRmin = 0. ; 
    G4double pRmax = 100. ; 
    G4double pSPhi = phi_start*pi ;    
    G4double pDPhi = phi_delta*pi ; 
    G4double pSTheta = 0. ; 
    G4double pDTheta = pi ;     // pi: full in theta

    return new G4Sphere(pName, pRmin, pRmax, pSPhi, pDPhi, pSTheta, pDTheta ); 
}






const G4VSolid* X4SolidMaker::SphereWithThetaSegment(const char* name)  // static
{
    double theta_start = SSys::getenvfloat("X4SolidMaker_SphereWithThetaSegment_theta_start", 0.f) ;  // units of pi
    double theta_delta = SSys::getenvfloat("X4SolidMaker_SphereWithThetaSegment_theta_delta", 0.5f) ; // units of pi

    LOG(info)
        << " (inputs are scaled by pi) "
        << " theta_start  " << theta_start
        << " theta_delta  " << theta_delta
        << " theta_start+theta_delta  " << theta_start+theta_delta
        << " theta_start*pi " << theta_start*pi
        << " theta_delta*pi " << theta_delta*pi
        << " (theta_start+theta_delta)*pi " << (theta_start+theta_delta)*pi
        ;

    G4String pName = name ; 
    G4double pRmin = 0. ; 
    G4double pRmax = 100. ; 
    G4double pSPhi = 0.*pi ;    
    G4double pDPhi = 2.*pi ; 
    G4double pSTheta = theta_start*pi ; 
    G4double pDTheta = theta_delta*pi ;  // theta_delta 1. : full in theta  

    return new G4Sphere(pName, pRmin, pRmax, pSPhi, pDPhi, pSTheta, pDTheta ); 
}


const G4VSolid* X4SolidMaker::AdditionAcrylicConstruction(const char* name)
{
    G4VSolid*   simple             = nullptr ;
    G4VSolid*   solidAddition_down = nullptr ;
    G4VSolid*   solidAddition_up   = nullptr ;
    G4VSolid*   uni_acrylic1       = nullptr ;

    double ZNodes3[3];
    double RminNodes3[3];
    double RmaxNodes3[3];
    ZNodes3[0] = 5.7*mm; RminNodes3[0] = 0*mm; RmaxNodes3[0] = 450.*mm;
    ZNodes3[1] = 0.0*mm; RminNodes3[1] = 0*mm; RmaxNodes3[1] = 450.*mm;
    ZNodes3[2] = -140.0*mm; RminNodes3[2] = 0*mm; RmaxNodes3[2] = 200.*mm;

    bool replace_poly = true ; 
    bool skip_sphere = true ; 

    simple = new G4Tubs("simple", 0*mm, 450*mm, 200*mm, 0.0*deg,360.0*deg);
    solidAddition_down = replace_poly ? simple : new G4Polycone("solidAddition_down",0.0*deg,360.0*deg,3,ZNodes3,RminNodes3,RmaxNodes3);

    solidAddition_up = new G4Sphere("solidAddition_up",0*mm,17820*mm,0.0*deg,360.0*deg,0.0*deg,180.*deg);
    uni_acrylic1 = skip_sphere ? solidAddition_down : new G4SubtractionSolid("uni_acrylic1",solidAddition_down,solidAddition_up,0,G4ThreeVector(0*mm,0*mm,+17820.0*mm));

    G4VSolid*   solidAddition_up1 = nullptr ;
    G4VSolid*   solidAddition_up2 = nullptr ;
    G4VSolid*   uni_acrylic2      = nullptr ;
    G4VSolid*   uni_acrylic3      = nullptr ;

    G4VSolid*   uni_acrylic2_initial = nullptr ;

    //double zshift = -20*mm ; 
    double zshift = 0*mm ; 

    solidAddition_up1 = new G4Tubs("solidAddition_up1",120*mm,208*mm,15.2*mm,0.0*deg,360.0*deg);
    uni_acrylic2 = new G4SubtractionSolid("uni_acrylic2",uni_acrylic1,solidAddition_up1,0,G4ThreeVector(0.*mm,0.*mm,zshift));
    uni_acrylic2_initial = uni_acrylic2 ; 


    solidAddition_up2 = new G4Tubs("solidAddition_up2",0,14*mm,52.5*mm,0.0*deg,360.0*deg);

    for(int i=0;i<8;i++)
    {
    uni_acrylic3 = new G4SubtractionSolid("uni_acrylic3",uni_acrylic2,solidAddition_up2,0,G4ThreeVector(164.*cos(i*pi/4)*mm,164.*sin(i*pi/4)*mm,-87.5));
    uni_acrylic2 = uni_acrylic3;
    }


    LOG(info)
        << " solidAddition_down " << solidAddition_down
        << " solidAddition_up " << solidAddition_up
        << " solidAddition_up1 " << solidAddition_up1
        << " solidAddition_up2 " << solidAddition_up2
        << " uni_acrylic2_initial " << uni_acrylic2_initial
        << " uni_acrylic1 " << uni_acrylic1 
        << " uni_acrylic2 " << uni_acrylic2
        << " uni_acrylic3 " << uni_acrylic3
        ;   

    //return solidAddition_down ;  // union of cylinder and cone
    //return solidAddition_up1 ;     // pipe cylinder 
    //return uni_acrylic1 ; 
    return uni_acrylic2_initial ; 
}



/**
X4SolidMaker::XJfixtureConstruction
-------------------------------------

solidXJfixture             Union
   down_uni4               Union
      down_uni3            Union
         down_uni2         Union
             down_uni1     Union
                down1      Tubs
                down2      Box       52.*mm, 0.*mm, 0.*mm
             down2         Box      -52.*mm, 0.*mm, 0.*mm
         down3             Box       0.*mm,  50.*mm, 0.*mm
      down3                Box       0.*mm, -50.*mm, 0.*mm 
   up_uni                  Union     
      up1                  Box
      up2                  Box       0.*mm, 0.*mm, 13.5*mm     (Up in Z)

**/

const int X4SolidMaker::XJfixtureConstruction_debug_mode = SSys::getenvint("X4SolidMaker__XJfixtureConstruction_debug_mode", 0 ) ; 
const G4VSolid* X4SolidMaker::XJfixtureConstruction(const char* name)
{
    G4VSolid* solidXJfixture_down1;
    G4VSolid* solidXJfixture_down2;
    G4VSolid* solidXJfixture_down3;
    G4VSolid* solidXJfixture_down_uni1;
    G4VSolid* solidXJfixture_down_uni2;
    G4VSolid* solidXJfixture_down_uni3;
    G4VSolid* solidXJfixture_down_uni4;
    //G4VSolid* solidXJfixture_down_uni5;

    G4VSolid* solidXJfixture_up1;
    G4VSolid* solidXJfixture_up2;
    G4VSolid* solidXJfixture_up_uni;

    G4VSolid* solidXJfixture;


// fixture part
    solidXJfixture_down1 = new G4Tubs("solidXJfixture_down1", 25.*mm, 45.*mm, 13./2*mm, 0.*deg, 360.*deg);
    solidXJfixture_down2 = new G4Box("solidXJfixture_down2", 10.*mm, 11.5*mm, 13/2.*mm);
    solidXJfixture_down_uni1 = new G4UnionSolid("solidXJfixture_down_uni1", solidXJfixture_down1, solidXJfixture_down2, 0, G4ThreeVector(52.*mm, 0.*mm, 0.*mm));
    solidXJfixture_down_uni2 = new G4UnionSolid("solidXJfixture_down_uni2", solidXJfixture_down_uni1, solidXJfixture_down2, 0, G4ThreeVector(-52.*mm, 0.*mm, 0.*mm));
    solidXJfixture_down3 = new G4Box("solidXJfixture_down3", 15.*mm, 15.*mm, 13/2.*mm);
    solidXJfixture_down_uni3 = new G4UnionSolid("solidXJfixture_down_uni3", solidXJfixture_down_uni2, solidXJfixture_down3, 0, G4ThreeVector(0.*mm, 50.*mm, 0.*mm));
    solidXJfixture_down_uni4 = new G4UnionSolid("solidXJfixture_down_uni4", solidXJfixture_down_uni3, solidXJfixture_down3, 0, G4ThreeVector(0.*mm, -50.*mm, 0.*mm));

    // down_uni4 is celtic-cross shape or uniform z half-thickness 13./2. = 6.5 mm  (shifts in x and y,  not z)

// cover part
    solidXJfixture_up1 = new G4Box("solidXJfixture_up1", 15.*mm, 40.*mm, 17/2.*mm);    
    solidXJfixture_up2 = new G4Box("solidXJfixture_up2", 15.*mm, 65*mm, 5.*mm);
    solidXJfixture_up_uni = new G4UnionSolid("solidXJfixture_up_uni", solidXJfixture_up1, solidXJfixture_up2, 0, G4ThreeVector(0.*mm, 0.*mm, 13.5*mm));

    //G4VSolid* new_solidXJfixture_up_uni = Uncoincide_Box_Box_Union( solidXJfixture_up_uni );
    //solidXJfixture_up_uni = new_solidXJfixture_up_uni ;


    /**
     up_uni is two box altar   : GETTING SPURIOUS INTERSECTS BECAUSE OF FLUSH UNION 

     TOFIX: 
         expand "up1" (as its smaller in y) by uncoincide/2 in hz 
         and shift it upwards to maintain the low edge at same place 

         BUT: "up1" is on LHS of the combination, "up2" is the one thats shifted up

         HMM: seems have to switch that around ... this is going to have knockon effect
         over what line is zero in Z which would cause a change in the placement 


         up2 is raised by 13.5 to form the thinner in z table top of the altar 

             
                                                           35    50     65        
                                                     
                                                            :     :     :                                altar frame              fixture frame 
                                                        
             -------------+                             +---+---+-+-----+        - - - - - - - - - - - - 18.5+13  =   31.5             6.5  
             |            |                             |   :   |      13/2=6.5                                       
             +            +                             +   :   + :     :         - - - - - - - - - - -  18.5+6.5 =   25               0.0           
             |            |                             |   :   |       :
             +------------+----------------+-----25-----+---20--+-+-----+         - - - - - - - - - -      8.5+10 =  18.5              -6.5  
             |                                                          |
             +    up2                      +                            +       - - - - - - - - - - - - -   8.5+5  = 13.5              -11.5
             |                                                          |
             +---------+^^^^^^^^^^^^^^^^^^^+^^^^^^^^^^^^^^^^^^+---------+       - - - - - - - - - - - - -             8.5              -16.5
                       |                                      |         
                       |                                     17/2=8.5        
                       +  up1                                 +                - - - - - - - - - - - - -              0.0              -25.0
                       |                                      |
                       |                                      |
               Z       +-------------------+-------40---------+            - - - - - - - - - - - - - - - -           -8.5              -33.5
               |                                    
               +-- Y
              /
             X


       Then altar is offset by -25. pushing its top down to 18.5 - 25. = -6.5 in final frame 
       which is flush with the lower edge of the celtic cross 
     **/   


// union 
    solidXJfixture = new G4UnionSolid("solidXJfixture", solidXJfixture_down_uni4, solidXJfixture_up_uni, 0, G4ThreeVector(0.*mm, 0.*mm, -25.*mm));


    G4VSolid* solidXJfixture_twiddle = new G4UnionSolid("solidXJfixture_twiddle", solidXJfixture_up_uni, solidXJfixture_down_uni4, 0, G4ThreeVector(0.*mm, 0.*mm, 25.*mm));
    // twiddling puts the zero at the altar frame zero 
    // so would have to offset the placement 



    G4VSolid* celtic_cross_sub_altar = new G4SubtractionSolid("solidXJfixture_celtic_cross_sub_altar", solidXJfixture_down_uni4, solidXJfixture_up_uni, 0, G4ThreeVector(0.*mm, 0.*mm, -25.*mm));


    G4VSolid* solidXJfixture_split = new G4UnionSolid("solidXJfixture_split", solidXJfixture_down_uni4, solidXJfixture_up_uni, 0, G4ThreeVector(0.*mm, 0.*mm, -50.*mm));




    G4VSolid* solid = solidXJfixture ;
    int debug_mode = XJfixtureConstruction_debug_mode ; 

    if( debug_mode > 0 )
    {
        switch(debug_mode)
        {
           case  0: solid = solidXJfixture           ; break ;    
           case  1: solid = solidXJfixture_down1     ; break ;    
           case  2: solid = solidXJfixture_down2     ; break ;    
           case  3: solid = solidXJfixture_down_uni1 ; break ;    
           case  4: solid = solidXJfixture_down_uni2 ; break ;    
           case  5: solid = solidXJfixture_down3     ; break ;    
           case  6: solid = solidXJfixture_down_uni3 ; break ;    
           case  7: solid = solidXJfixture_down_uni4 ; break ;    
           case  8: solid = solidXJfixture_up1       ; break ;    
           case  9: solid = solidXJfixture_up2       ; break ;    
           case 10: solid = solidXJfixture_up_uni    ; break ;    
           case 11: solid = celtic_cross_sub_altar   ; break ;    
           case 12: solid = solidXJfixture_split     ; break ;    
           case 13: solid = solidXJfixture_twiddle   ; break ;    
        } 
        LOG(info) 
            << "X4SolidMaker__XJfixtureConstruction_debug_mode " << debug_mode
            << " solid.GetName " << ( solid ? solid->GetName() : "-" )
            ; 
        assert(solid); 
    }
    return solid ;
}

/**
X4SolidMaker::Uncoincide_Box_Box_Union
----------------------------------------

To avoid coincidence need to expand the smaller box into the larger 
without changing the position the lower edge.
Hence increase the half-size in Z from *hz* to *new_hz* and 
simultaneously shift upwards by the same amount (*zoff*) 
to keep the lower edge at same z position::


       +hz + uncoincide - - - - - - - +~~~~~~~~~+ - -    zoff + new_hz  - - -
                                      |         |
                                      |         |
       +hz  +---------+ - - - - - - - | - - - - | - - - - - - - - - - - - - -
            |         |               |         |  
            |         |               |         |
            |         |               |         |
            |         |               |_________|        zoff 
            |         |               |         |
        0 --|---------| - - - - - - - - - - - - - - - - - - - - - - - - - - -
            |         |               |         |
            |         |               |         |
            |         |               |         |                                
            |         |               |         |
            |         |               |         |
       -hz  +---------+ - - - - - - - +---------+ - - -  zoff - new_hz  - - -



Line equations::

      hz + uncoincide = zoff + new_hz

      -hz             = zoff - new_hz 

Add them::

     zoff = uncoincide/2

Subtract them::
     
     new_hz = hz + uncoincide/2 





up2 is raised by 13.5 to form the thinner in z table top of the altar 


         +---------------------------+     5mm                     - - -  8.5 + 10 = 18.5
         |         up2               |  - - - -   13.5  = 8.5+5
         +-----+---------------+-----+
               |               |   17/2 = 8.5mm       
               |   up1         |   - - - -    
               |               |
               +---------------+ 
                                          10 mm thin top of altar, 
                                          17 mm thicker bottom of altar




HMM: the sign of the change to the translation depends on 
whether the smaller_box (which needs to grow into the larger) is on 
the rhs of the combination which has the translation applied to it 


**/

G4VSolid* X4SolidMaker::Uncoincide_Box_Box_Union( const G4VSolid* bbu  )  // static
{
    LOG(info) << " bbu.GetName " << bbu->GetName() ;  

    const G4BooleanSolid* bs = dynamic_cast<const G4BooleanSolid*>(bbu) ; 

    const G4VSolid* a = bs->GetConstituentSolid(0) ; 
    const G4VSolid* _b = bs->GetConstituentSolid(1) ; 
    const G4DisplacedSolid* _b_disp = dynamic_cast<const G4DisplacedSolid*>(_b) ; 
    G4ThreeVector b_tla = _b_disp->GetObjectTranslation();
    const G4VSolid* b = _b_disp->GetConstituentMovedSolid()  ; 

    LOG(info) << " a.GetName " << a->GetName() ;  
    LOG(info) << " _b.GetName " << _b->GetName() ;  
    LOG(info) << " _b_disp.GetName " << _b_disp->GetName() ;  
    LOG(info) << " b.GetName " << b->GetName() ;  
    LOG(info) << " b_tla " << Desc(&b_tla) ;
 
    const G4Box* a_box = dynamic_cast<const G4Box*>(a); 
    const G4Box* b_box = dynamic_cast<const G4Box*>(b); 
    LOG(info) << " a_box " << Desc(a_box) ;  
    LOG(info) << " b_box " << Desc(b_box) ;  

    G4ThreeVector new_tla(b_tla); 

    std::string new_name = bs->GetName()  ; 

    G4Box* new_a = new G4Box(a->GetName(), a_box->GetXHalfLength(), a_box->GetYHalfLength(), a_box->GetZHalfLength() ); 
    G4Box* new_b = new G4Box(b->GetName(), b_box->GetXHalfLength(), b_box->GetYHalfLength(), b_box->GetZHalfLength() ); 

    int shift_axis = OneAxis(&b_tla); 
    LOG(info) << " shift_axis " << shift_axis ; 



    enum { A, B, UNKNOWN } ; 
    int smaller = UNKNOWN ; 
  
    for(int axis=0 ; axis < 3 ; axis++)
    {
        if(axis == shift_axis) continue ;   // 
        double ah = HalfLength(a_box, axis); 
        double bh = HalfLength(b_box, axis); 
        if(ah == bh) continue ;    // ignore equal axes 
        smaller = ah < bh ? A : B ; 
    } 

    LOG(info) << " smaller " << smaller ; 


    double uncoincide = 1.*mm ; 
    if(smaller != UNKNOWN )
    {
        G4Box* smaller_box = smaller == A ? new_a : new_b ; 
        LOG(info) << " smaller_box.GetName " << smaller_box->GetName() ; 

        ChangeBoxHalfLength( smaller_box, shift_axis, uncoincide/2. );         

        ChangeThreeVector(   &new_tla ,   shift_axis, uncoincide/2. ) ;     
    }
    else
    {
        LOG(fatal) << " failed to uncoincide " ; 
    }

    G4UnionSolid* new_bbu = new G4UnionSolid( new_name, new_a, new_b, 0, new_tla ); 
    return new_bbu ; 
}


double X4SolidMaker::HalfLength( const G4Box* box, int axis )
{
    double value = 0. ; 
    switch(axis)
    {
        case X: value = box->GetXHalfLength() ; break ; 
        case Y: value = box->GetYHalfLength() ; break ; 
        case Z: value = box->GetZHalfLength() ; break ; 
    }
    return value ; 
}

void X4SolidMaker::ChangeThreeVector( G4ThreeVector* v, int axis, double delta )
{
    if( v == nullptr ) return ; 
    switch(axis)
    {
        case X: v->setX(v->x() + delta)  ; break ; 
        case Y: v->setY(v->y() + delta)  ; break ; 
        case Z: v->setZ(v->z() + delta)  ; break ; 
    }
}
void X4SolidMaker::ChangeBoxHalfLength( G4Box* box, int axis, double delta )
{
    switch(axis)
    {
        case X: box->SetXHalfLength(box->GetXHalfLength() + delta)  ; break ; 
        case Y: box->SetYHalfLength(box->GetYHalfLength() + delta)  ; break ; 
        case Z: box->SetZHalfLength(box->GetZHalfLength() + delta)  ; break ; 
    }
}






int X4SolidMaker::OneAxis( const G4ThreeVector* v )
{
    double x = v ? v->x() : 0. ; 
    double y = v ? v->y() : 0. ; 
    double z = v ? v->z() : 0. ; 
    int axis = ERR ; 
    if( x != 0. && y == 0. && z == 0. ) axis = X ; 
    if( x == 0. && y != 0. && z == 0. ) axis = Y ; 
    if( x == 0. && y == 0. && z != 0. ) axis = Z ; 
    return axis ; 
}



std::string X4SolidMaker::Desc( const G4Box* box )
{
    std::stringstream ss ; 
    ss 
       << "(" 
       << std::fixed << std::setw(10) << std::setprecision(3) << box->GetXHalfLength() << " "
       << std::fixed << std::setw(10) << std::setprecision(3) << box->GetYHalfLength() << " "
       << std::fixed << std::setw(10) << std::setprecision(3) << box->GetZHalfLength()
       << ") "
       << box->GetName()
       ;
    std::string s = ss.str();
    return s ; 
}

std::string X4SolidMaker::Desc( const G4ThreeVector* v )
{
    std::stringstream ss ; 
    ss 
       << "(" 
       << std::fixed << std::setw(10) << std::setprecision(3) << (v ? v->x() : 0. ) << " "
       << std::fixed << std::setw(10) << std::setprecision(3) << (v ? v->y() : 0. )  << " "
       << std::fixed << std::setw(10) << std::setprecision(3) << (v ? v->z() : 0. ) 
       << ") "
       ;
    std::string s = ss.str();
    return s ; 
} 




/**
X4SolidMaker::XJanchorConstruction
-----------------------------------

Observed spurious Geant4 intersects on the line between the Tubs and the Cons::

    solidXJanchor          G4UnionSolid

        sub                G4SubtractionSolid      (subtract big sphere from cone)  
              down         G4Cons
              ball         G4Sphere

        up                 G4Tubs

**/

const G4VSolid* X4SolidMaker::XJanchorConstruction(const char* name)
{
    G4VSolid* solidXJanchor_up;
    G4VSolid* solidXJanchor_down;
    G4VSolid* solidXJanchor_ball;

    solidXJanchor_up   = new G4Tubs("solidXJanchor_up", 0.*mm, 25.*mm, 13./2*mm, 0.*deg, 360.*deg);
    solidXJanchor_down = new G4Cons("solidXJanchor_down", 0.*mm, 47.*mm, 0.*mm, 73.*mm, 10.*mm, 0.*deg, 360.*deg);   // to subtract the ball
    solidXJanchor_ball = new G4Sphere("solidXJanchor_ball", 0.*mm, 17820.*mm, 0.*deg, 360.*deg, 0.*deg, 180.*deg); 
    G4SubtractionSolid* solidXJanchor_sub = new G4SubtractionSolid("solidXJanchor_sub",solidXJanchor_down, solidXJanchor_ball, 0, G4ThreeVector(0.*mm, 0*mm,  17820.*mm));
    G4UnionSolid* solidXJanchor = new G4UnionSolid("solidXJanchor",solidXJanchor_sub, solidXJanchor_up, 0, G4ThreeVector(0.*mm, 0*mm,-16.5*mm));

    return solidXJanchor ;
} 


const G4VSolid* X4SolidMaker::SJReceiverConstruction(const char* name)
{
    G4VSolid* solidSJReceiver_up ;
    G4VSolid* solidSJReceiver_down ;
    G4VSolid* solidSJReceiver_box ;
    G4VSolid* solidSJReceiver_ball ;  

    //solidSJReceiver_up   = new G4Cons("solidSJReceiver_up", 0.*mm, 31.7*mm, 0.*mm, 25*mm, 13./2*mm, 0.*deg, 360.0*deg);
    solidSJReceiver_up = new G4Tubs("solidXJanchor_up", 0.*mm, 25.*mm, 13./2*mm, 0.*deg, 360.*deg);
    solidSJReceiver_down = new G4Cons("solidSJReceiver_down", 0.*mm, 73.*mm, 0.*mm, 47.*mm, 10.*mm, 0.*deg, 360.*deg);   // to subtract the ball
    //solidSJReceiver_down = new G4Cons("solidSJReceiver_down", 0.*mm, 47.*mm, 0.*mm, 60.*mm, 5.*mm, 0.*deg, 360.*deg); // original size
    solidSJReceiver_box = new G4Box("solidSJReceiver_box", 17780.*mm, 17780.*mm, 17780.*mm);
    solidSJReceiver_ball = new G4Sphere("solidSJReceiver_ball", 0.*mm, 17700.*mm, 0.*deg, 360.*deg, 0.*deg, 180.*deg); 
    //G4SubtractionSolid* solidSphere_sub = new G4SubtractionSolid("solidSphere_sub", solidSJReceiver_box, solidSJReceiver_ball);
    G4SubtractionSolid* solidSJReceiver_sub = new G4SubtractionSolid("solidSJReceiver_sub",solidSJReceiver_down, solidSJReceiver_ball, 0, G4ThreeVector(0.*mm, 0*mm,  17699.938*mm));
    G4UnionSolid* solidSJReceiver = new G4UnionSolid("solidSJReceiver",solidSJReceiver_sub, solidSJReceiver_up, 0, G4ThreeVector(0.*mm, 0*mm,16.5*mm));
    return solidSJReceiver ;
}



const G4VSolid* X4SolidMaker::BoxMinusTubs0(const char* name)  // is afflicted
{
    double tubs_hz = 15.2*mm ;   
    double zshift = 0*mm ; 
    G4VSolid* box   = new G4Box("box",  250*mm, 250*mm, 100*mm ); 
    G4VSolid* tubs =  new G4Tubs("tubs",120*mm,208*mm,tubs_hz,0.0*deg,360.0*deg);
    G4VSolid* box_minus_tubs = new G4SubtractionSolid(name,box,tubs,0,G4ThreeVector(0.*mm,0.*mm,zshift));  
    return box_minus_tubs ; 
}

const G4VSolid* X4SolidMaker::BoxMinusTubs1(const char* name) 
{
    double tubs_hz = 15.2*mm ;   
    G4VSolid* box   = new G4Box("box",  250*mm, 250*mm, 100*mm ); 
    G4VSolid* tubs =  new G4Tubs("tubs",120*mm,208*mm,tubs_hz,0.0*deg,360.0*deg);
    G4VSolid* box_minus_tubs = new G4SubtractionSolid(name,box,tubs);  
    return box_minus_tubs ; 
}

const G4VSolid* X4SolidMaker::BoxMinusOrb(const char* name) 
{
    double radius = SSys::getenvfloat("X4SolidMaker_BoxMinusOrb_radius", 100.f) ; 

    double sx     = SSys::getenvfloat("X4SolidMaker_BoxMinusOrb_sx", 100.f) ; 
    double sy     = SSys::getenvfloat("X4SolidMaker_BoxMinusOrb_sy", 100.f) ; 
    double sz     = SSys::getenvfloat("X4SolidMaker_BoxMinusOrb_sz", 100.f) ; 

    double dx     = SSys::getenvfloat("X4SolidMaker_BoxMinusOrb_dx", 0.f) ;  
    double dy     = SSys::getenvfloat("X4SolidMaker_BoxMinusOrb_dy", 0.f) ;  
    double dz     = SSys::getenvfloat("X4SolidMaker_BoxMinusOrb_dz", 0.f) ;  

    G4VSolid* box = new G4Box("box",  sx, sy, sz ); 
    G4VSolid* orb = new G4Orb("orb",  radius ); 

    G4VSolid* box_minus_orb = new G4SubtractionSolid(name,box,orb,nullptr, G4ThreeVector(dx, dy, dz) );  
    return box_minus_orb ; 
}


const G4VSolid* X4SolidMaker::PolyconeWithMultipleRmin(const char* name)
{ 
    double ZUpper4[4];
    double RminUpper4[4];
    double RmaxUpper4[4];
    ZUpper4[0] = 0*mm;   RminUpper4[0] = 43.*mm; RmaxUpper4[0] = 195.*mm;
    ZUpper4[1] = -15*mm; RminUpper4[1] = 43.*mm; RmaxUpper4[1] = 195.*mm;
    ZUpper4[2] = -15*mm; RminUpper4[2] = 55.5*mm; RmaxUpper4[2] = 70.*mm;
    ZUpper4[3] = -101*mm; RminUpper4[3] = 55.5*mm; RmaxUpper4[3] = 70.*mm;
    
    G4VSolid* base_steel = new G4Polycone("base_steel",0.0*deg,360.0*deg,4,ZUpper4,RminUpper4,RmaxUpper4);
    return base_steel ;
}


const G4VSolid* X4SolidMaker::UnionOfHemiEllipsoids(const char* name )
{
    assert( strstr( name, "UnionOfHemiEllipsoids" ) != nullptr ); 

    std::vector<long> vals ; 
    Extract(vals, name); 
    long iz = vals.size() > 0 ? vals[0] : 0 ; 

    std::cout 
        << "X4SolidMaker::UnionOfHemiEllipsoids"
        << " name " << name 
        << " vals.size " << vals.size()
        << " iz " << iz
        << std::endl 
        ; 


    double rx = 150. ; 
    double ry = 150. ; 
    double rz = 100. ; 

    double z2 = rz ; 
    double z1 = 0. ; 
    double z0 = -rz ; 

    G4VSolid* upper = new G4Ellipsoid("upper", rx, ry, rz, z1, z2 );  
    G4VSolid* lower = new G4Ellipsoid("lower", rx, ry, rz, z0, z1 );  

    G4VSolid* solid = nullptr ; 
    if( iz == 0 )
    {
        solid = new G4UnionSolid(name, upper, lower );   
    }
    else
    {
        double zoffset = double(iz) ; 
        G4ThreeVector tlate(0., 0., zoffset) ; 
        solid = new G4UnionSolid(name, upper, lower, nullptr, tlate );   
    }
    return solid ; 
}


void X4SolidMaker::Extract( std::vector<long>& vals, const char* s )  // static
{
    char* s0 = strdup(s); 
    char* p = s0 ; 
    while (*p) 
    {   
        if( (*p >= '0' && *p <= '9') || *p == '+' || *p == '-') vals.push_back(strtol(p, &p, 10)) ; 
        else p++ ;
    }   
    free(s0); 
}




