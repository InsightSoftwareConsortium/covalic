
/********************************************************************************

Abstract class that defines properties of image metrics:
whether it requires binary input images, the best and worst scores, etc

********************************************************************************/

#ifndef _AbstractValidationMetric_h
#define _AbstractValidationMetric_h

class AbstractValidationMetric
{
public:

  // Defines whether input contains binary data
  virtual bool IsInputBinary()  = 0;

  // Defines whether metric is symmetric f(A,B) = f(B,A)
  virtual bool IsSymmetric()  = 0;

  // Value returned when the match is best
  // Ex: 1 for Dice, 0 for surface distance
  virtual double GetBestScore()  = 0;

  // Value returned when the match is worst
  // Ex: 0 for Dice, Inf for surface distance
  virtual double GetWorstScore()  = 0;

};

#endif
