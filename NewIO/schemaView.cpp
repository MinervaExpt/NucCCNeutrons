//Problem: For the one Analysis in this job, I want to specify a list of branches to cache for each event
//         that is unique to this Analysis.  I think I can add features to this approach to only sometimes
//         fill the cache later.
//Ideas: Sounds like dynamic dispatch.  I know the branches I want to read at compile-time, but I don't know
//       which ones I'll use.  I only have to do this once per event, so it's probably not quite as bad as
//       TreeWrapper's lookups.
//
//       Ways to make a decision at runtime:
//       virtual functions
//       function pointer
//       std::function<>
//       dynamic dispatch
//
//Question: Can I use double dispatch to set up my cache differently for each Analysis?  Nope.  Some base class would have to be
//          updated with a new overload of a method for each new derived class.  That defeats the purpose of a compile-time factory.
//
//Idea: I want to look up branches only once per file (when the TTree is loaded) and then maintain them in a location mapped to
//      an identifier at compile-time.  I'm hoping this will remove the lookup overhead in TreeWrapper.  Does this mean that I
//      need a central "schema" class that has places to map all possible branches?

//TODO: Making BranchPtr perform branch lookup only the first time it's used instead of in its
//      constructor greatly mitigates my schema evolution problem.  Analyses that still use the
//      same branches they had when a given AnaTuple was produced will always be able to analyze
//      that AnaTuple.  New study-specific branches won't get in the way.

//A Schema maps branches that need to be looked up to locations in memory that are fixed at compile-time.
//I plan to create one Schema each time a new TTree is opened.
//Nota Bene: You could even implement Schema itself with ChainWrapper!
class Schema
{
  public:
    Schema(const TreeHandle& handle); //Set up the BranchPtr<>s.

    //TODO: These could be implemented with std::function<> instead to not type the same name
    //      twice but produce a very weird inheritance system.
    BranchPtr<double> q3{"NucCCNeutrons_q3"};
    BranchPtr<std::vector<int>> fsPDGs{"truth_FS_PDG_codes"};
};

//A SchemaView is a lightweight view onto the branches of a Schema.
//It could be implemented by caching, changing, or reading those values
//from a TTree.
class SchemaView
{
  public:
    virtual double q3() = 0;
    virtual std::vector<int> fsPDGs() = 0;
};

//Worst-case Schema access: read the branches every time
class NotCached: public SchemaView
{
  public:
    NotCached(const Schema& schema): fSchema(schema) {}

    virtual double q3() override { return fSchema.q3(); }
    virtual std::vector<int> fsPDGs() override { return fSchema.fsPDGs(); }

  private:
    Schema& fSchema; //Branches from which all values will come
};

//A Universe is a SchemaView that shifts values from another SchemaView.
//SchemaView could even have non-virtual methods that use its virtual
//methods to construct more convenient objects like NeutronCandidates.
class Universe: public SchemaView
{
  public:
    Universe() = default; //fCV is nullptr

    void setCache(const SchemaView& cv) { fCV = &cv; }

    //TODO: I have to implement SchemaView's functions here!
    //      This means that there are 4 places where new branches have to be
    //      maintained: Schema, SchemaView, NotCached, and Universe
    //
    //      I want Universe and CACHEs to access values in different
    //      ways, so I think I have to have 4 points of maintenance.
    virtual double q3() override { return fCV->q3(); }
    virtual std::vector<int> fsPDGs() override { return fCV->fsPDGs(); }

  private:
    SchemaView* fCV; //Observer pointer to SchemaView where central value numbers come from before shift
};

//Systematic Universe based on Ben's systematics framework.
class Q3Shift: public Universe
{
  public:
    Q3Shift(const SchemaView& cv, const YAML::Node& config): Universe(cv), nSigma(config.as<int>("nSigma"))
    {
    }

    virtual double q3() override
    {
      return SchemaView::q3() * nSigma;
    }

  private:
    const int nSigma;  //How many sigma to shift q3 by
};

//Base class for plot-making plugins.  Gives the compile-time
//plugin system something to hold on to.
class Analysis
{
  public:
    Analysis();

    //Cache data that will be read for every universe into a custom SchemaView.
    //This will be the argument to analyze() later.  If you don't override this
    //function, your analysis will default to reading every value it uses from
    //a TTree for every Universe.
    virtual SchemaView& cache(const Schema& scehma) { return NotCached(schema); }

    //Fill plots based on this event.
    virtual void analyze(const SchemaView& event) = 0;
};

//Example Analysis plugin.  Selected at runtime.
//This is the code the user needs to write.
class ReadsFS: public Analysis
{
  public:
    //Define the values this Analysis needs to read from branches.
    class Cache: public NotCached
    {
      public:
        Cache(const Schema& schema): NotCached(schema)
        {
        }

        virtual double q3() override { return q3; }
        virtual std::vector<int> fsPDGs() override { return fsPDGs; }

      private:
        //Cached branch values
        double q3;
        std::vector<int> fsPDGs;
    };

    virtual SchemaView& cache(const Schema& schema) override
    {
      return Cache(schema);
    }

    virtual void analyze(const SchemaView& event) override
    {
      //Do things with the numbers in event...
    }
};

//Now, my event loop could look like...
for(const auto& name: fileNames)
{
  std::unique_ptr<TFile> file(TFile::Open(name.c_str())); //If this fails, the ROOT error message should throw an exception
  auto tree = file->Get(treeName.c_str()); //TODO: Will this trigger a ROOT error message?

  //Make sure this is really the same MC/data status as the last file
  //and other sanity checks...

  Schema schema(*tree); //Branch lookup happens here
  size_t entry = 0;
  while(tree->GetEntry(entry++)) //With ROOT throwing exceptions on I/O error messages, I think it's safe to
                                 //just check for nonzero bytes read.
  {
    //analysis is a plugin chosen by a YAML file

    //Process the CV
    NotCached readFromTree(schema);
    const SchemaView& cache = readFromTree; //Don't force a read to start with
    bool filledCache = false;
    //TODO: Make sure cuts are run in the order they appear in the YAML file.
    if(std::all_of(cuts.begin(), cuts.end(), [&cache](const auto& cut) { return cut(cache); })) //TODO: Caching for cuts?
    {
      //No need to check for whether cache has been filled.  It has to be empty at this point.
      cache = analysis->cache(schema); //Cache values once for all universes to use
      filledCache = true;
      analysis->analyze(cache);

      //Process vertical error bands.  They only provide weights and don't actually shift values.
      for(const auto& vert: verticalUniverses)
      {
        vert->setCache(cache);
        analysis->analyze(*vert);
      } //For each vertical systematic universe
    } //If CV passes all cuts

    //Process lateral error bands.  They may pass cuts independent of the CV.
    for(const auto& lateral: lateralUniverses) //Each univ is a plugin populated from a YAML file
    {
      lateral->setCache(cache); //Use the cached values if I've already cached them anyway.
                                //Otherwise, cuts read directly from tree.
      if(std::all_of(cuts.begin(), cuts.end(), [&lateral](const auto& cut) { return cut(*lateral); })) //TODO: Caching for cuts?
      {
        //Fill univCache the first time a lateral universe passes all cuts.
        //This should reduce the amount of data read from file and thus encourage
        //the OS to cache bits of this file for rapid iteration.
        if(!filledCache)
        {
          cache = analysis->cache(schema);
          lateral->setCache(cache);
          filledCache = true;
        } //If no universe has passed all cuts so far

        analysis->analyze(*lateral);
      } //If this univ passes all Cuts
    } //For each lateral systematic universe
  } //For each entry in tree
} //For each file name
