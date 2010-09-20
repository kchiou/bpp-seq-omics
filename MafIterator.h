//
// File: MafIterator.h
// Authors: Julien Dutheil
// Created: Tue Sep 07 2010
//

/*
Copyright or © or Copr. Bio++ Development Team, (2010)

This software is a computer program whose purpose is to provide classes
for sequences analysis.

This software is governed by the CeCILL  license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/

#ifndef _MAFITERATOR_H_
#define _MAFITERATOR_H_

#include "../SequenceWithAnnotation.h"
#include "../SequenceTools.h"
#include "../Alphabet/AlphabetTools.h"
#include "../Container/AlignedSequenceContainer.h"

//From the STL:
#include <iostream>
#include <string>
#include <deque>

namespace bpp {

/**
 * @brief A sequence class which is used to store data from MAF files.
 * 
 * It extends the SequenceWithAnnotation class to store MAF-specific features,
 * like the chromosome position. The sequence is its own listener,
 * and recomputes its "genomic" site by using the SequenceTools::getNumberOfSites
 * function when a content modification is performed.
 * Tags like begin and stop, hovever, have to be set by hand.
 *
 * A MAF sequence is necessarily a DNA sequence.
 */
class MafSequence:
  public SequenceWithAnnotation
{
  private:
    bool         hasCoordinates_;
    unsigned int begin_;
    std::string  species_;
    std::string  chromosome_;
    char         strand_;
    unsigned int size_;
    unsigned int srcSize_;

  public:
    MafSequence():
      SequenceWithAnnotation(&AlphabetTools::DNA_ALPHABET), hasCoordinates_(false), begin_(0), species_(""), chromosome_(""), strand_(0), size_(0), srcSize_(0)
    {
      size_ = 0;
    }

    MafSequence(const std::string& name, const std::string& sequence):
      SequenceWithAnnotation(name, sequence, &AlphabetTools::DNA_ALPHABET), hasCoordinates_(false), begin_(0), species_(""), chromosome_(""), strand_(0), size_(0), srcSize_(0)
    {
      size_ = SequenceTools::getNumberOfSites(*this);
      size_t pos = name.find(".");
      if (pos != std::string::npos) {
        chromosome_ = name.substr(pos + 1);
        species_    = name.substr(0, pos);
      }
    }

    MafSequence(const std::string& name, const std::string& sequence, unsigned int begin, char strand, unsigned int srcSize) :
      SequenceWithAnnotation(name, sequence, &AlphabetTools::DNA_ALPHABET), hasCoordinates_(begin > 0), begin_(begin), species_(""), chromosome_(""), strand_(strand), size_(0), srcSize_(srcSize)
    {
      size_ = SequenceTools::getNumberOfSites(*this);
      size_t pos = name.find(".");
      if (pos != std::string::npos) {
        chromosome_ = name.substr(pos + 1);
        species_    = name.substr(0, pos);
      }
    }

    MafSequence* clone() const { return new MafSequence(*this); }

    ~MafSequence() {}

  public:
    bool hasCoordinates() const { return hasCoordinates_; }

    void removeCoordinates() { hasCoordinates_ = false; begin_ = 0; }

    unsigned int start() const throw (Exception) { 
      if (hasCoordinates_) return begin_;
      else throw Exception("MafSequence::start(). Sequence does not have coordinates.");
    }

    unsigned int stop() const { 
      if (hasCoordinates_) return begin_ + size_ - 1;
      else throw Exception("MafSequence::stop(). Sequence does not have coordinates.");
    }

    const std::string& getSpecies() const { return species_; }
    
    const std::string& getChromosome() const { return chromosome_; }
    
    char getStrand() const { return strand_; }
    
    unsigned int getGenomicSize() const { return size_; }
    
    unsigned int getSrcSize() const { return srcSize_; }
    
    void setStart(unsigned int begin) { begin_ = begin; hasCoordinates_ = true; }
    
    void setChromosome(const std::string& chr) { chromosome_ = chr; }
    
    void setStrand(char s) { strand_ = s; }
    
    void setSrcSize(unsigned int srcSize) { srcSize_ = srcSize; }
  
    std::string getDescription() const { return getName() + strand_ + ":" + TextTools::toString(start()) + "-" + TextTools::toString(stop()); }
  
    MafSequence* subSequence(unsigned int startAt, unsigned int length) const;

  private:
    void beforeSequenceChanged(const SymbolListEditionEvent& event) {}
    void afterSequenceChanged(const SymbolListEditionEvent& event) { size_ = SequenceTools::getNumberOfSites(*this); }
    void beforeSequenceInserted(const SymbolListInsertionEvent& event) {}
    void afterSequenceInserted(const SymbolListInsertionEvent& event) { size_ = SequenceTools::getNumberOfSites(*this); }
    void beforeSequenceDeleted(const SymbolListDeletionEvent& event) {}
    void afterSequenceDeleted(const SymbolListDeletionEvent& event) { size_ = SequenceTools::getNumberOfSites(*this); }
    void beforeSequenceSubstituted(const SymbolListSubstitutionEvent& event) {}
    void afterSequenceSubstituted(const SymbolListSubstitutionEvent& event) {}
};

/**
 * @brief A synteny block data structure, the basic unit of a MAF alignement file.
 *
 * This class basically contains a AlignedSequenceContainer made of MafSequence objects.
 */
class MafBlock
{
  private:
    double score_;
    unsigned int pass_;
    AlignedSequenceContainer alignment_;

  public:
    MafBlock() :
      score_(-1),
      pass_(0),
      alignment_(&AlphabetTools::DNA_ALPHABET)
    {}

  public:
    void setScore(double score) { score_ = score; }
    void setPass(unsigned int pass) { pass_ = pass; }
    
    double getScore() const { return score_; }
    unsigned int getPass() const { return pass_; }

    AlignedSequenceContainer& getAlignment() { return alignment_; }
    const AlignedSequenceContainer& getAlignment() const { return alignment_; }

    unsigned int getNumberOfSequences() const { return alignment_.getNumberOfSequences(); }
    
    unsigned int getNumberOfSites() const { return alignment_.getNumberOfSites(); }

    void addSequence(const MafSequence& sequence) { alignment_.addSequence(sequence, false); }

    const MafSequence& getSequence(const std::string& name) const throw (SequenceNotFoundException) {
      return dynamic_cast<const MafSequence&>(getAlignment().getSequence(name));
    }

    const MafSequence& getSequence(unsigned int i) const throw (IndexOutOfBoundsException) {
      return dynamic_cast<const MafSequence&>(getAlignment().getSequence(i));
    }

    //Return the first sequence with the species name.
    const MafSequence& getSequenceForSpecies(const std::string& species) const throw (SequenceNotFoundException) {
      for (unsigned int i = 0; i < getNumberOfSequences(); ++i) {
        const MafSequence* seq = &getSequence(i);
        if (seq->getSpecies() == species)
          return *seq;
      }
      throw SequenceNotFoundException("MafBlock::getSequenceForSpecies. No sequence with the given species name in this block.");
    }

    void removeCoordinatesFromSequence(unsigned int i) throw (IndexOutOfBoundsException) {
      //This is a bit of a trick, but avoid useless recopies.
      //It is safe here because the AlignedSequenceContainer is fully encapsulated.
      //It would not work if a VectorSiteContainer was used.
      const_cast<MafSequence&>(getSequence(i)).removeCoordinates();
    }


};


/**
 * @brief Interface to loop over maf alignment blocks.
 */
class MafIterator
{
  public:
    virtual ~MafIterator() {}

  public:
    /**
     * @brief Get the next available alignment block.
     *
     * @return A maf alignment block, or a null pointer if no more block is available.
     */
    virtual MafBlock* nextBlock() throw (Exception) = 0;
    
};

/**
 * @brief Interface to loop over removed blocks of a maf alignment.
 */
class MafTrashIterator
{
  public:
    virtual ~MafTrashIterator() {}

  public:
    /**
     * @brief Get the next available removed alignment block.
     *
     * @return A maf alignment block, or a null pointer if no more block is available.
     */
    virtual MafBlock* nextRemovedBlock() throw (Exception) = 0;
    
};


/**
 * @brief Helper class for developping filter for maf blocks.
 */
class AbstractFilterMafIterator:
  public MafIterator
{
  protected:
    MafIterator* iterator_;
    OutputStream* logstream_;
    bool verbose_;

  public:
    AbstractFilterMafIterator(MafIterator* iterator) :
      iterator_(iterator), logstream_(ApplicationTools::message), verbose_(true) {}

  private:
    AbstractFilterMafIterator(const AbstractFilterMafIterator& it): iterator_(it.iterator_), logstream_(it.logstream_), verbose_(it.verbose_) {}
    AbstractFilterMafIterator& operator=(const AbstractFilterMafIterator& it) {
      iterator_  = it.iterator_;
      logstream_ = it.logstream_;
      verbose_   = it.verbose_;
      return *this;
    }

  public:
    void setLogStream(OutputStream* logstream) { logstream_ = logstream; }
    bool verbose() const { return verbose_; }
    void verbose(bool yn) { verbose_ = yn; }

};

/**
 * @brief Filter maf blocks to keep only the ones with a minimum number of sites.
 */
class BlockSizeMafIterator:
  public AbstractFilterMafIterator
{
  private:
    unsigned int minSize_;

  public:
    BlockSizeMafIterator(MafIterator* iterator, unsigned int minSize) :
      AbstractFilterMafIterator(iterator),
      minSize_(minSize)
    {}

  public:
    MafBlock* nextBlock() throw (Exception) {
      MafBlock* block;
      bool test;
      do {
        block = iterator_->nextBlock();
        if (!block) return 0;
        test = (block->getNumberOfSites() < minSize_);
        if (test) {
          if (logstream_) {
            (*logstream_ << "BLOCK SIZE FILTER: block with size " << block->getNumberOfSites() << " was discarded.").endLine();
          }
          delete block;
        }
      } while (test);
      return block;
    }

};

/**
 * @brief Filter maf blocks to keep a subset of sequences, given their name.
 *
 * Blocks that are empty after the filtering are removed.
 */
class SequenceFilterMafIterator:
  public AbstractFilterMafIterator
{
  private:
    std::vector<std::string> species_;
    bool strict_;
    bool rmDuplicates_;
    MafBlock* currentBlock_;

  public:
    /**
     * @param strict If true, then block that do not contain all species will be discarded.
     * @param rmDuplicates If true, block that contain more than one instance for at least one species will be discarded.
     */
    SequenceFilterMafIterator(MafIterator* iterator, const std::vector<std::string>& species, bool strict = false, bool rmDuplicates = false) :
      AbstractFilterMafIterator(iterator),
      species_(species),
      strict_(strict),
      rmDuplicates_(rmDuplicates),
      currentBlock_(0)
    {}

  private:
    SequenceFilterMafIterator(const SequenceFilterMafIterator& iterator) :
      AbstractFilterMafIterator(0),
      species_(iterator.species_),
      strict_(iterator.strict_),
      rmDuplicates_(iterator.rmDuplicates_),
      currentBlock_(0)
    {}
    
    SequenceFilterMafIterator& operator=(const SequenceFilterMafIterator& iterator)
    {
      species_       = iterator.species_;
      strict_        = iterator.strict_;
      rmDuplicates_  = iterator.rmDuplicates_;
      currentBlock_  = 0;
      return *this;
    }

  public:
    MafBlock* nextBlock() throw (Exception);

};

/**
 * @brief Merge blocks if some of their sequences are contiguous.
 *
 * The user specifies the focus species. Sequences that are not in this set will
 * be merged without testing, and their genomic coordinates removed.
 * The scores, if any, will be averaged for the block, weighted by the corresponding block sizes.
 * the pass value will be removed if it is different for the two blocks.
 * It is possible to define a maximum distance for the merging. Setting a distance of zero implies that the blocks
 * have to be exactly contiguous. Alternatively, the appropriate number of 'N' will be inserted in all species.
 * All species however have to be distant of the exact same amount.
 */
class BlockMergerMafIterator:
  public AbstractFilterMafIterator
{
  private:
    std::vector<std::string> species_;
    MafBlock* incomingBlock_;
    MafBlock* currentBlock_;
    std::vector<std::string> ignoreChrs_; //These chromsomes will never be merged (ex: 'Un').
    unsigned int maxDist_;

  public:
    BlockMergerMafIterator(MafIterator* iterator, const std::vector<std::string>& species, unsigned int maxDist = 0) :
      AbstractFilterMafIterator(iterator),
      species_(species),
      incomingBlock_(0),
      currentBlock_(0),
      ignoreChrs_(),
      maxDist_(maxDist)
    {
      incomingBlock_ = iterator->nextBlock();
    }

  private:
    BlockMergerMafIterator(const BlockMergerMafIterator& iterator) :
      AbstractFilterMafIterator(0),
      species_(iterator.species_),
      incomingBlock_(iterator.incomingBlock_),
      currentBlock_(iterator.currentBlock_),
      ignoreChrs_(iterator.ignoreChrs_),
      maxDist_(iterator.maxDist_)
    {}
    
    BlockMergerMafIterator& operator=(const BlockMergerMafIterator& iterator)
    {
      species_       = iterator.species_;
      incomingBlock_ = iterator.incomingBlock_;
      currentBlock_  = iterator.currentBlock_;
      ignoreChrs_    = iterator.ignoreChrs_;
      maxDist_       = iterator.maxDist_;
      return *this;
    }

  public:
    MafBlock* nextBlock() throw (Exception);

    /**
     * brief Add a chromosome that should be ignored to the list.
     * @param chr The name of the chromosome to be ignored.
     */
    void ignoreChromosome(const std::string& chr) {
      ignoreChrs_.push_back(chr);
    }
};

/**
 * @brief Filter maf blocks to remove in each block the positions made only of gaps.
 *
 * The subset of species that should be examined is given as input. The coordinates of these
 * species will not be altered as only gap positions are removed. Other species however may be
 * altered as they might not have gap in the removed position. The coordinates for these species
 * will therefore be removed as they do not make sense anymore.
 */
class FullGapFilterMafIterator:
  public AbstractFilterMafIterator
{
  private:
    std::vector<std::string> species_;

  public:
    FullGapFilterMafIterator(MafIterator* iterator, const std::vector<std::string>& species) :
      AbstractFilterMafIterator(iterator),
      species_(species)
    {}

  public:
    MafBlock* nextBlock() throw (Exception);

};

/**
 * @brief Filter maf blocks to remove ambiguously aligned or non-informative regions.
 *
 * Regions with a too high proportion of gaps or unknown character in a set of species will be removed,
 * and blocks adjusted accordingly. 
 */
class AlignmentFilterMafIterator:
  public AbstractFilterMafIterator,
  public MafTrashIterator
{
  private:
    std::vector<std::string> species_;
    unsigned int windowSize_;
    unsigned int step_;
    unsigned int maxGap_;
    std::deque<MafBlock*> blockBuffer_;
    std::deque<MafBlock*> trashBuffer_;
    std::deque< std::vector<bool> > window_;
    bool keepTrashedBlocks_;

  public:
    AlignmentFilterMafIterator(MafIterator* iterator, const std::vector<std::string>& species, unsigned int windowSize, unsigned int step, unsigned int maxGap, bool keepTrashedBlocks) :
      AbstractFilterMafIterator(iterator),
      species_(species),
      windowSize_(windowSize),
      step_(step),
      maxGap_(maxGap),
      blockBuffer_(),
      trashBuffer_(),
      window_(species.size()),
      keepTrashedBlocks_(keepTrashedBlocks)
    {}

  public:
    MafBlock* nextBlock() throw (Exception);

    MafBlock* nextRemovedBlock() throw (Exception) {
      if (trashBuffer_.size() == 0) return 0;
      MafBlock* block = trashBuffer_.front();
      trashBuffer_.pop_front();
      return block;
    }

};

/**
 * @brief Filter maf blocks to remove regions with masked positions.
 *
 * Regions with a too high proportion of masked position in a set of species will be removed,
 * and blocks adjusted accordingly. 
 */
class MaskFilterMafIterator:
  public AbstractFilterMafIterator,
  public MafTrashIterator
{
  private:
    std::vector<std::string> species_;
    unsigned int windowSize_;
    unsigned int step_;
    unsigned int maxMasked_;
    std::deque<MafBlock*> blockBuffer_;
    std::deque<MafBlock*> trashBuffer_;
    std::deque< std::vector<bool> > window_;
    bool keepTrashedBlocks_;

  public:
    MaskFilterMafIterator(MafIterator* iterator, const std::vector<std::string>& species, unsigned int windowSize, unsigned int step, unsigned int maxMasked, bool keepTrashedBlocks) :
      AbstractFilterMafIterator(iterator),
      species_(species),
      windowSize_(windowSize),
      step_(step),
      maxMasked_(maxMasked),
      blockBuffer_(),
      trashBuffer_(),
      window_(species.size()),
      keepTrashedBlocks_(keepTrashedBlocks)
    {}

  public:
    MafBlock* nextBlock() throw (Exception);

    MafBlock* nextRemovedBlock() throw (Exception) {
      if (trashBuffer_.size() == 0) return 0;
      MafBlock* block = trashBuffer_.front();
      trashBuffer_.pop_front();
      return block;
    }

};

/**
 * @brief Filter maf blocks to remove regions with low quality.
 *
 * Regions with a too low average quality in a set of species will be removed,
 * and blocks adjusted accordingly. 
 */
class QualityFilterMafIterator:
  public AbstractFilterMafIterator,
  public MafTrashIterator
{
  private:
    std::vector<std::string> species_;
    unsigned int windowSize_;
    unsigned int step_;
    unsigned int minQual_;
    std::deque<MafBlock*> blockBuffer_;
    std::deque<MafBlock*> trashBuffer_;
    std::deque< std::vector<int> > window_;
    bool keepTrashedBlocks_;

  public:
    QualityFilterMafIterator(MafIterator* iterator, const std::vector<std::string>& species, unsigned int windowSize, unsigned int step, unsigned int minQual, bool keepTrashedBlocks) :
      AbstractFilterMafIterator(iterator),
      species_(species),
      windowSize_(windowSize),
      step_(step),
      minQual_(minQual),
      blockBuffer_(),
      trashBuffer_(),
      window_(species.size()),
      keepTrashedBlocks_(keepTrashedBlocks)
    {}

  public:
    MafBlock* nextBlock() throw (Exception);

    MafBlock* nextRemovedBlock() throw (Exception) {
      if (trashBuffer_.size() == 0) return 0;
      MafBlock* block = trashBuffer_.front();
      trashBuffer_.pop_front();
      return block;
    }

};


class TrashIteratorAdapter:
  public MafIterator
{
  private:
    MafTrashIterator* iterator_;

  public:
    TrashIteratorAdapter(MafTrashIterator* iterator) :
      iterator_(iterator) {}

  private:
    TrashIteratorAdapter(const TrashIteratorAdapter& iterator) :
      iterator_(iterator.iterator_) {}
    
    TrashIteratorAdapter& operator=(const TrashIteratorAdapter& iterator) {
      iterator_ = iterator.iterator_;
      return *this;
    }

  public:
    MafBlock* nextBlock() throw (Exception) {
      return iterator_->nextRemovedBlock();
    }
};

/**
 * @brief This iterator forward the iterator given as input after having printed its content to a file.
 */
class OutputMafIterator:
  public AbstractFilterMafIterator
{
  private:
    std::ostream* output_;
    bool mask_;

  public:
    OutputMafIterator(MafIterator* iterator, std::ostream* out, bool mask = true) :
      AbstractFilterMafIterator(iterator), output_(out), mask_(mask)
    {
      if (output_)
        writeHeader(*output_);
    }

  private:
    OutputMafIterator(const OutputMafIterator& iterator) :
      AbstractFilterMafIterator(0),
      output_(iterator.output_),
      mask_(iterator.mask_)
    {}
    
    OutputMafIterator& operator=(const OutputMafIterator& iterator)
    {
      output_ = iterator.output_;
      mask_   = iterator.mask_;
      return *this;
    }


  public:
    MafBlock* nextBlock() throw (Exception) {
      MafBlock* block = iterator_->nextBlock();
      if (output_ && block)
        writeBlock(*output_, *block);
      return block;
    }

  private:
    void writeHeader(std::ostream& out) const;
    void writeBlock(std::ostream& out, const MafBlock& block) const;
};

/**
 * @brief This special iterator synchronizes two adaptors.
 *
 * It takes as input a main iterator and a secondary one. The nextBlock method of the secondary iterator will be
 * called immediately after the one of the primary one. The resulting block of the main iterator will be forwarded,
 * while the one of the secondary iterator will be destroyed.
 */
class MafIteratorSynchronizer:
  public AbstractFilterMafIterator
{
  private:
    MafIterator* secondaryIterator_;

  public:
    MafIteratorSynchronizer(MafIterator* primaryIterator, MafIterator* secondaryIterator) :
      AbstractFilterMafIterator(primaryIterator), secondaryIterator_(secondaryIterator)
    {}

  private:
    MafIteratorSynchronizer(const MafIteratorSynchronizer& iterator) :
      AbstractFilterMafIterator(0),
      secondaryIterator_(iterator.secondaryIterator_)
    {}
    
    MafIteratorSynchronizer& operator=(const MafIteratorSynchronizer& iterator)
    {
      secondaryIterator_ = iterator.secondaryIterator_;
      return *this;
    }


  public:
    MafBlock* nextBlock() throw (Exception) {
      MafBlock* block = iterator_->nextBlock();
      MafBlock* secondBlock = secondaryIterator_->nextBlock();
      if (secondBlock)
        delete secondBlock;
      return block;
    }

};


} // end of namespace bpp.

#endif //_MAFITERATOR_H_
