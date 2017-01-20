/**********************************************************************
 ** This program is part of 'MOOSE', the
 ** Multiscale Object Oriented Simulation Environment.
 **           copyright (C) 2003-2008
 **           Upinder S. Bhalla, Niraj Dudani and NCBS
 ** It is made available under the terms of the
 ** GNU Lesser General Public License version 2.1
 ** See the file COPYING.LIB for the full notice.
 ****/

#include "header.h"
#include "DifShellBase.h"
#include "ElementValueFinfo.h"

SrcFinfo1< double >* DifShellBase::concentrationOut()
{
  static SrcFinfo1< double > concentrationOut("concentrationOut",
					      "Sends out concentration");
  return &concentrationOut;
}

SrcFinfo2< double, double >* DifShellBase::innerDifSourceOut(){
  static SrcFinfo2< double, double > sourceOut("innerDifSourceOut",
					       "Sends out source information.");
  return &sourceOut;
}

SrcFinfo2< double, double >* DifShellBase::outerDifSourceOut(){
  static SrcFinfo2< double, double > sourceOut("outerDifSourceOut",
					       "Sends out source information.");
  return &sourceOut;
}

const Cinfo* DifShellBase::initCinfo()
{
    
  static DestFinfo process( "process",
			    "Handles process call",
			    new ProcOpFunc< DifShellBase>(&DifShellBase::process ) );
  static DestFinfo reinit( "reinit",
			   "Reinit happens only in stage 0",
			   new ProcOpFunc< DifShellBase>( &DifShellBase::reinit ));
    
  static Finfo* processShared[] = {
    &process, &reinit
  };

  static SharedFinfo proc(
			  "proc", 
			  "Shared message to receive Process message from scheduler",
			  processShared, sizeof( processShared ) / sizeof( Finfo* ));
    
  static DestFinfo reaction( "reaction",
			     "Here the DifShell receives reaction rates (forward and backward), and concentrations for the "
			     "free-buffer and bound-buffer molecules.",
			     new EpFunc4< DifShellBase, double, double, double, double >( &DifShellBase::buffer ));
    
  static Finfo* bufferShared[] = {
    DifShellBase::concentrationOut(), &reaction
  };
  static SharedFinfo buffer( "buffer",
			     "This is a shared message from a DifShell to a Buffer (FixBuffer or DifBuffer). "
			     "During stage 0:\n "
			     " - DifShell sends ion concentration\n"
			     " - Buffer updates buffer concentration and sends it back immediately using a call-back.\n"
			     " - DifShell updates the time-derivative ( dC / dt ) \n"
			     "\n"
			     "During stage 1: \n"
			     " - DifShell advances concentration C \n\n"
			     "This scheme means that the Buffer does not need to be scheduled, and it does its computations when "
			     "it receives a cue from the DifShell. May not be the best idea, but it saves us from doing the above "
			     "computations in 3 stages instead of 2." ,
			     bufferShared,
			     sizeof( bufferShared ) / sizeof( Finfo* ));

  static DestFinfo fluxFromOut( "fluxFromOut",
				"Destination message",
				new EpFunc2< DifShellBase, double, double > ( &DifShellBase::fluxFromOut ));
 
  static Finfo* innerDifShared[] = {
    &fluxFromOut,  DifShellBase::innerDifSourceOut(),
  };
  static SharedFinfo innerDif( "innerDif",
			       "This shared message (and the next) is between DifShells: adjoining shells exchange information to "
			       "find out the flux between them. "
			       "Using this message, an inner shell sends to, and receives from its outer shell." ,
			       innerDifShared,
			       sizeof( innerDifShared ) / sizeof( Finfo* ));

  static DestFinfo fluxFromIn( "fluxFromIn", "",
			       new EpFunc2< DifShellBase, double, double> ( &DifShellBase::fluxFromIn ) );
    
  static Finfo* outerDifShared[] = {
    &fluxFromIn,  DifShellBase::outerDifSourceOut(),
  };

  static  SharedFinfo outerDif( "outerDif",
				"Using this message, an outer shell sends to, and receives from its inner shell." ,
				outerDifShared,
				sizeof( outerDifShared ) / sizeof( Finfo* ));
 
  static ElementValueFinfo< DifShellBase, double> C( "C", 
						 "Concentration C",// is computed by the DifShell",
						 &DifShellBase::setC,
						 &DifShellBase::getC);
  static ElementValueFinfo< DifShellBase, double> Ceq( "Ceq", "",
						   &DifShellBase::setCeq,
						   &DifShellBase::getCeq);
  static ElementValueFinfo< DifShellBase, double> D( "D", "",
						 &DifShellBase::setD,
						 &DifShellBase::getD);
  static ElementValueFinfo< DifShellBase, double> valence( "valence", "",
						       &DifShellBase::setValence,
						       &DifShellBase::getValence);
  static ElementValueFinfo< DifShellBase, double> leak( "leak", "",
						    &DifShellBase::setLeak,
						    &DifShellBase::getLeak);
  static ElementValueFinfo< DifShellBase, unsigned int> shapeMode( "shapeMode", "",
							       &DifShellBase::setShapeMode,
							       &DifShellBase::getShapeMode);
  static ElementValueFinfo< DifShellBase, double> length( "length", "",
						      &DifShellBase::setLength,
						      &DifShellBase::getLength);
  static ElementValueFinfo< DifShellBase, double> diameter( "diameter", "",
							&DifShellBase::setDiameter,
							&DifShellBase::getDiameter );
  static ElementValueFinfo< DifShellBase, double> thickness( "thickness", "",
							 &DifShellBase::setThickness,
							 &DifShellBase::getThickness );
  static ElementValueFinfo< DifShellBase, double> volume( "volume", "",
						      &DifShellBase::setVolume,
						      &DifShellBase::getVolume );
  static ElementValueFinfo< DifShellBase, double> outerArea( "outerArea", "",
							 &DifShellBase::setOuterArea,
							 &DifShellBase::getOuterArea);
  static ElementValueFinfo< DifShellBase, double> innerArea( "innerArea", "",
							 &DifShellBase::setInnerArea,
							 &DifShellBase::getInnerArea );

    
  static DestFinfo influx( "influx", "",
			   new EpFunc1< DifShellBase, double > (&DifShellBase::influx ));
  static DestFinfo outflux( "outflux", "",
			    new EpFunc1< DifShellBase, double >( &DifShellBase::influx ));
  static DestFinfo fInflux( "fInflux", "",
			    new EpFunc2< DifShellBase, double, double >( &DifShellBase::fInflux ));
  static DestFinfo fOutflux( "fOutflux", "",
			     new EpFunc2< DifShellBase, double, double> (&DifShellBase::fOutflux ));
  static DestFinfo storeInflux( "storeInflux", "",
				new EpFunc1< DifShellBase, double >( &DifShellBase::storeInflux ) );
  static DestFinfo storeOutflux( "storeOutflux", "",
				 new EpFunc1< DifShellBase, double > ( &DifShellBase::storeOutflux ) );
  static DestFinfo tauPump( "tauPump","",
			    new EpFunc2< DifShellBase, double, double >( &DifShellBase::tauPump ) );
  static DestFinfo eqTauPump( "eqTauPump", "",
			      new EpFunc1< DifShellBase, double >( &DifShellBase::eqTauPump ) );
  static DestFinfo mmPump( "mmPump", "",
			   new EpFunc2< DifShellBase, double, double >( &DifShellBase::mmPump ) );
  static DestFinfo hillPump( "hillPump", "",
			     new EpFunc3< DifShellBase, double, double, unsigned int >( &DifShellBase::hillPump ) );
  static Finfo* difShellBaseFinfos[] = {
    //////////////////////////////////////////////////////////////////
    // Field definitions
    //////////////////////////////////////////////////////////////////
    &C,
    &Ceq,
    &D,
    &valence,
    &leak,
    &shapeMode,
    &length,
    &diameter,
    &thickness,
    &volume,
    &outerArea,
    &innerArea,
    //////////////////////////////////////////////////////////////////
    // MsgSrc definitions
    //////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////
    // SharedFinfo definitions
    //////////////////////////////////////////////////////////////////
    &proc,
    &buffer,
    // concentrationOut(),
    //innerDifSourceOut(),
    //outerDifSourceOut(),
    &innerDif,
    &outerDif,
    //////////////////////////////////////////////////////////////////
    // DestFinfo definitions
    //////////////////////////////////////////////////////////////////
    &influx,
    &outflux,
    &fInflux,
    &fOutflux,
    &storeInflux,
    &storeOutflux,
    &tauPump,
    &eqTauPump,
    &mmPump,
    &hillPump,
  };

  static string doc[] =
    {
      "Name", "DifShellBase",
      "Author", "Niraj Dudani. Ported to async13 by Subhasis Ray/Asia Jedrzejewska-Szmek",
      "Description", "DifShell object: Models diffusion of an ion (typically calcium) within an "
      "electric compartment. A DifShell is an iso-concentration region with respect to "
      "the ion. Adjoining DifShells exchange flux of this ion, and also keep track of "
      "changes in concentration due to pumping, buffering and channel currents, by "
      "talking to the appropriate objects.",
    };
  static Dinfo< DifShellBase> dinfo;
  static Cinfo difShellBaseCinfo(
			     "DifShellBase",
			     Neutral::initCinfo(),
			     difShellBaseFinfos,
			     sizeof( difShellBaseFinfos ) / sizeof( Finfo* ),
			     &dinfo,
			     doc,
			     sizeof( doc ) / sizeof( string ));

  return &difShellBaseCinfo;
}

//Cinfo *object*  corresponding to the class.
static const Cinfo* difShellBaseCinfo = DifShellBase::initCinfo();

DifShellBase::DifShellBase() :
  shapeMode_( 0 ),
  length_( 0.0 ),
  diameter_( 0.0 ),
  thickness_( 0.0 ),
  volume_( 0.0 ),
  outerArea_( 0.0 ),
  innerArea_( 0.0 )
{ ; }

void DifShellBase::setC( const Eref& e, double C)
{
  vSetC(e,C);
}
double DifShellBase::getC(const Eref& e) const
{
  return vGetC(e);
}

void DifShellBase::setCeq( const Eref& e, double Ceq )
{
  vSetCeq(e,C);
}

double DifShellBase::getCeq(const Eref& e ) const
{
  return vGetCeq(e);
}

void DifShellBase::setD(const Eref& e, double D )
{
  vSetD(e,D)
}

double DifShellBase::getD(const Eref& e ) const
{
  return vGetD(e);
}

void DifShellBase::setValence(const Eref& e, double valence )
{
  vSetValence(e,valence);
}

double DifShellBase::getValence(const Eref& e ) const 
{
  return vGetValence(e);
}

void DifShellBase::setLeak(const Eref& e, double leak )
{
  vSetLeak(e,leak);
}

double DifShellBase::getLeak(const Eref& e ) const
{
  return vGetLeak(e);
}

void DifShellBase::setShapeMode(const Eref& e, unsigned int shapeMode )
{
  if ( shapeMode != 0 && shapeMode != 1 && shapeMode != 3 ) {
    cerr << "Error: DifShell: I only understand shapeModes 0, 1 and 3.\n";
    return;
  }
  shapeMode_ = shapeMode;
}

unsigned int DifShellBase::getShapeMode(const Eref& e) const
{
  return shapeMode_;
}

void DifShellBase::setLength(const Eref& e, double length )
{
  if ( length < 0.0 ) {
    cerr << "Error: DifShell: length cannot be negative!\n";
    return;
  }
	
  length_ = length;
}

double DifShellBase::getLength(const Eref& e ) const
{
  return length_;
}

void DifShellBase::setDiameter(const Eref& e, double diameter )
{
  if ( diameter < 0.0 ) {
    cerr << "Error: DifShell: diameter cannot be negative!\n";
    return;
  }
	
  diameter_ = diameter;
}

double DifShellBase::getDiameter(const Eref& e ) const
{
  return diameter_;
}

void DifShellBase::setThickness( const Eref& e, double thickness )
{
  if ( thickness < 0.0 ) {
    cerr << "Error: DifShell: thickness cannot be negative!\n";
    return;
  }
	
  thickness_ = thickness;
}

double DifShellBase::getThickness(const Eref& e) const
{
  return thickness_;
}

void DifShellBase::setVolume(const Eref& e, double volume )
{
  if ( shapeMode_ != 3 )
    cerr << "Warning: DifShell: Trying to set volume, when shapeMode is not USER-DEFINED\n";
	
  if ( volume < 0.0 ) {
    cerr << "Error: DifShell: volume cannot be negative!\n";
    return;
  }
	
  volume_ = volume;
}

double DifShellBase::getVolume(const Eref& e ) const
{
  return volume_;
}

void DifShellBase::setOuterArea(const Eref& e, double outerArea )
{
  if (shapeMode_ != 3 )
    cerr << "Warning: DifShell: Trying to set outerArea, when shapeMode is not USER-DEFINED\n";
	
  if ( outerArea < 0.0 ) {
    cerr << "Error: DifShell: outerArea cannot be negative!\n";
    return;
  }
	
  outerArea_ = outerArea;
}

double DifShellBase::getOuterArea(const Eref& e ) const
{
  return outerArea_;
}

void DifShellBase::setInnerArea(const Eref& e, double innerArea )
{
  if ( shapeMode_ != 3 )
    cerr << "Warning: DifShell: Trying to set innerArea, when shapeMode is not USER-DEFINED\n";
    
  if ( innerArea < 0.0 ) {
    cerr << "Error: DifShell: innerArea cannot be negative!\n";
    return;
  }
    
  innerArea_ = innerArea;
}

double DifShellBase::getInnerArea(const Eref& e) const
{
  return innerArea_;
}

////////////////////////////////////////////////////////////////////////////////
// Dest functions
////////////////////////////////////////////////////////////////////////////////

void DifShellBase::reinit( const Eref& e, ProcPtr p )
{
  vReinit( e, p );
}

void DifShellBase::process( const Eref& e, ProcPtr p )
{
  vProcess( e, p );
}

void DifShellBase::buffer(
		      const Eref& e,
		      double kf,
		      double kb,
		      double bFree,
		      double bBound )
{
  vBuffer( e, kf, kb, bFree, bBound );
}

void DifShellBase::fluxFromOut(const Eref& e,
			   double outerC,
			   double outerThickness )
{
  vFluxFromOut(e, outerC, outerThickness );
}

void DifShellBase::fluxFromIn(
			  const Eref& e,
			  double innerC,
			  double innerThickness )
{
  vFluxFromIn( e, innerC, innerThickness );
}

void DifShellBase::influx(const Eref& e,
		      double I )
{
  vInflux( e, I );
}

void DifShellBase::outflux(const Eref& e,
		       double I )
{
  vOutflux(e, I );
}

void DifShellBase::fInflux(const Eref& e,
		       double I,
		       double fraction )
{
  vFInflux(e, I, fraction );
}

void DifShellBase::fOutflux(const Eref& e,
			double I,
			double fraction )
{
  vFOutflux(e, I, fraction );
}

void DifShellBase::storeInflux(const Eref& e,
			   double flux )
{
  vStoreInflux( e, flux );
}

void DifShellBase::storeOutflux(const Eref& e,
			    double flux )
{
  vStoreOutflux(e, flux );
}

void DifShellBase::tauPump(const Eref& e,
		       double kP,
		       double Ceq )
{
  vTauPump( e, kP, Ceq );
}

void DifShellBase::eqTauPump(const Eref& e,
			 double kP )
{
  vEqTauPump(e, kP );
}

void DifShellBase::mmPump(const Eref& e,
		      double vMax,
		      double Kd )
{
  vMMPump(e, vMax, Kd );
}

void DifShellBase::hillPump(const Eref& e,
			double vMax,
			double Kd,
			unsigned int hill )
{
  vHillPump(e, vMax, Kd, hill );
}
void DifShellBase::vSetSolver( const Eref& e, Id hsolve )
{;}

void DifShellBase::zombify( Element* orig, const Cinfo* zClass, 
				Id hsolve )
{
	if ( orig->cinfo() == zClass )
		return;
	unsigned int start = orig->localDataStart();
	unsigned int num = orig->numLocalData();
	if ( num == 0 )
		return;
	unsigned int len = 12;
	vector< double > data( num * len );

	unsigned int j = 0;
	
	for ( unsigned int i = 0; i < num; ++i ) {
	  Eref er( orig, i + start );
	  const DifShell* ds = 
	    reinterpret_cast< const DifShell* >( er.data() );
	  data[j + 0] = ds->getC( er );
	  data[j + 1] = ds->getCeq( er );
	  data[j + 2] = ds->getD( er );
	  data[j + 3] = ds->getValence( er );
	  data[j + 4] = ds->getLeak( er );
	  data[j + 5] = ds->getShapeMode( er );
	  data[j + 6] = ds->getLength( er );
	  data[j + 7] = ds->getDiameter( er );
	  data[j + 8] = ds->getThickness( er );
	  data[j + 9] = ds->getVolume( er );
	  data[j + 10] = ds->getOuterArea( er );
	  data[j + 11] = ds->getInnerArea( er );
	  j += len;
	}
	orig->zombieSwap( zClass );
	j = 0;
	for ( unsigned int i = 0; i < num; ++i ) {
	  Eref er( orig, i + start );
	  const DifShell* ds = 
	    reinterpret_cast< const DifShell* >( er.data() );
	  ds->vSetSolver(er,hsolve);
	  ds->setC(er, data[j+0]);
	  ds->setCeq(er, data[j + 1]);
	  ds->setD(er, data[j + 2]);
	  ds->setValence(er, data[j + 3]);
	  ds->setLeak(er, data[j + 4]);
	  ds->setShapeMode(er, data[j + 5]);
	  ds->setLength(er, data[j + 6]);
	  ds->setDiameter(er, data[j + 7]);
	  ds->setThickness(er, data[j + 8]);
	  ds->setVolume(er, data[j + 9]);
	  ds->setOuterArea(er, data[j + 10]);
	  ds->setInnerArea(er, data[j + 11]);
	  j += len; //??
	}
	
}
