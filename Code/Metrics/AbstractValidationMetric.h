
#ifndef _AbstractValidationMetric_h
#define _AbstractValidationMetric_h

class AbstractValidationMetric
{
public:

  // Value returned when the match is best
  // Ex: 1 for Dice, 0 for surface distance
  double GetBestScore() = 0;

  // Value returned when the match is worst
  // Ex: 0 for Dice, Inf for surface distance
  double GetWorstScore() = 0;
};

#endif
