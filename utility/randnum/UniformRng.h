/*******************************************************************
 * File:            UniformRng.h
 * Description:      
 * Author:          Subhasis Ray
 * E-mail:          ray.subhasis@gmail.com
 * Created:         2008-02-01 11:52:48
 ********************************************************************/
/**********************************************************************
** This program is part of 'MOOSE', the
** Messaging Object Oriented Simulation Environment,
** also known as GENESIS 3 base code.
**           copyright (C) 2003-2005 Upinder S. Bhalla. and NCBS
** It is made available under the terms of the
** GNU General Public License version 2
** See the file COPYING.LIB for the full notice.
**********************************************************************/

#ifndef _UNIFORMRNG_H
#define _UNIFORMRNG_H

#include "randnum.h"
#include "basecode/header.h"
#include "basecode/moose.h"
#include "RandGenerator.h"
class UniformRng: public RandGenerator
{
  public:
    UniformRng();
    ~UniformRng();
    
    static double getMean(const Element*e );
    static double getVariance(const Element *e );
    static double getMin(const Element* e);
    static double getMax(const Element* e);
    static void setMin(const Conn& c, double min);
    static void setMax(const Conn& c, double max);
    static double getSample(const Element* e);
  private:
    double min_;
    double max_;    
};

    
    
#endif
