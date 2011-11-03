#include "NewDetectorContFact.h"

#include "NewDetectorGeoPar.h"

#include "FairRuntimeDb.h"

#include <iostream>

ClassImp(NewDetectorContFact)

static NewDetectorContFact gNewDetectorContFact;

NewDetectorContFact::NewDetectorContFact()
  : FairContFact()
{
  /** Constructor (called when the library is loaded) */
  fName="NewDetectorContFact";
  fTitle="Factory for parameter containers in libTutorial1";
  setAllContainers();
  FairRuntimeDb::instance()->addContFactory(this);
}

void NewDetectorContFact::setAllContainers()
{
  /** Creates the Container objects with all accepted
      contexts and adds them to
      the list of containers for the Tutorial1 library.
  */

  FairContainer* p= new FairContainer("NewDetectorGeoPar",
                                      "NewDetector Geometry Parameters",
                                      "TestDefaultContext");
  p->addContext("TestNonDefaultContext");

  containers->Add(p);
}

FairParSet* NewDetectorContFact::createContainer(FairContainer* c)
{
  /** Calls the constructor of the corresponding parameter container.
      For an actual context, which is not an empty string and not
      the default context
      of this container, the name is concatinated with the context.
  */
  const char* name=c->GetName();
  FairParSet* p=NULL;
  if (strcmp(name,"NewDetectorGeoPar")==0) {
    p=new NewDetectorGeoPar(c->getConcatName().Data(),
                            c->GetTitle(),c->getContext());
  }
  return p;
}