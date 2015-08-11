#pragma once

#ifndef DEBUG_TYPE 
#define DEBUG_TYPE "protect"
#endif

#include <set>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Debug.h>
#include <vector>

using namespace llvm;
// The types of protection. wake_lock, irq, spin_lock etc. are represented
// by unique "int" in following
typedef std::set<int> Protection;

// This will hold the protection summary of function, for example,
// this functions acquires wake_lock in all its paths or releases
// wake_lock in one of the paths (conservative)
class ProtectSummary {
	public:
		std::set<int> acquires;
		std::set<int> releases;

		//ProtectSummary(): acquires(), releases() {}

		// Will be called when we encounter a call instruction.
		// Return true if p changed
		bool applyThisSummary (Protection *p){
			Protection tmp (*p);
			//DEBUG( dbgs() << "applyThisSummary before \n");
			//for( Protection::iterator it = p->begin(), e = p->end(); it!=e; ++it){
				//DEBUG( dbgs() << *it);
			//}
			// Add all the acquires from the set
			p->insert( acquires.begin(), acquires.end() );
			// Remove all the releases from the set
			for( std::set<int>::iterator it = releases.begin(), e = releases.end();
						it!=e; ++it){
				p->erase( *it );
			}
			//DEBUG( dbgs() << "applyThisSummary after \n");
			//for( Protection::iterator it = p->begin(), e = p->end(); it!=e; ++it){
				//DEBUG( dbgs() << *it);
			//}
			bool ret = (tmp != *p);
			//DEBUG( dbgs() << "applyThisSummary " << ret << "\n");
			return ret;
		}

		bool applyPSummary ( ProtectSummary &ps ){
			ProtectSummary tmp;
			tmp.copyFrom( *this );
			//dbgs() << "applyPSummary " ;
			//ps.print(dbgs());
			//print(dbgs());
			//dbgs() << "\n...\n " ;

			// Remove all acquires (releases) that have been released (acquired) in ps
			for( std::set<int>::iterator it = ps.acquires.begin(), e = ps.acquires.end();
						it!=e; ++it){
				releases.erase( *it );
			}
			for( std::set<int>::iterator it = ps.releases.begin(), e = ps.releases.end();
						it!=e; ++it){
				acquires.erase( *it );
			}

			// Insert all acquires and releases in ps
			acquires.insert( ps.acquires.begin(), ps.acquires.end() );
			releases.insert( ps.releases.begin(), ps.releases.end() );

			return isEqual( tmp );
		}

		void copyFrom( const ProtectSummary C ){
			if( !C.acquires.empty() ){
				acquires = C.acquires;
			} 
			if( !C.releases.empty() ){
				releases = C.releases;
			}
		}

		void unionMany( const std::vector<ProtectSummary> U ){
			//DEBUG( dbgs() << "Enter unionMany \n" );
			if( U.empty()  ){
				return ;
			}

			std::vector<ProtectSummary>::const_iterator it = U.begin() ;
			copyFrom(*it);
			++it;

			for ( ; it != U.end(); ++it) {
				unionOne( *it ); 
			}

			// Remove all the acquires from the release. 
			for (std::set<int>::iterator i = acquires.begin(), e = acquires.end(); 
					i != e; ++i) {
				std::set<int>::iterator it = releases.find(*i);
				if( it != releases.end() ){
					acquires.erase( i ); 
				}
			}
			//DEBUG( dbgs() << "Exit unionMany \n" );
		}

		// To be called at a join. this = L union this
		void unionOne ( const ProtectSummary L ){
			//DEBUG( dbgs() << "Enter unionOne \n" );
			// If something was released in ANY of the branches, then it is 
			// considered as release - union
			releases.insert( L.releases.begin(), L.releases.end() );
			//DEBUG( dbgs() << "Done releases \n" );

			// If something was acquired in ALL of the branches, then it is 
			// considered as acquire - intersection
			for (std::set<int>::iterator i = acquires.begin(), e = acquires.end(); 
					i != e; ++i) {
				std::set<int>::iterator it = L.acquires.find(*i);
				if( it != L.acquires.end() ){
					acquires.erase( i ); 
				}
			}
			//DEBUG( dbgs() << "Exit unionOne \n" );
		}

		bool isEqual( ProtectSummary P ){
			bool ret = true;
			ret = ret && (acquires == P.acquires );
			ret = ret && (releases == P.releases );
			return ret;
		}
		
		bool changeProtection( bool isAcquire, int protectionId ){
			// hack test
			//acquires.insert(1);
			DEBUG( dbgs() << "changeProtection called\n");
			if( isAcquire ){
				releases.erase( protectionId );
				if( acquires.find( protectionId ) != acquires.end() )
					return false;
				acquires.insert( protectionId );
			} else{
				acquires.erase( protectionId );
				if( releases.find( protectionId ) != releases.end() )
					return false;
				releases.insert( protectionId );
			}
			return true;
		}

		void print(raw_ostream &ro){
			ro << "\n acquires:";
			for( std::set<int>::iterator it = acquires.begin(), e = acquires.end();
				it!=e; ++it){
				ro << *it;
			}
			ro << "\n releases:";
			for( std::set<int>::iterator it = releases.begin(), e = releases.end();
				it!=e; ++it){
				ro << *it;
			}
		}
};
