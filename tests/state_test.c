//////////////////////////////////////////////////////////////////
//
// Test για το state.h module
//
//////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "acutest.h"			// Απλή βιβλιοθήκη για unit testing
#include "ADTSet.h"
#include "state.h"
#include "set_utils.h"

int* create_inT(int value){
	int* p = malloc(sizeof(int));
	*p = value;
	return p;
}

int compare_ints( Pointer a, Pointer b ){
	return *(int*)a - *(int*)b;
}

void test_state_create() {

	State state = state_create();
	TEST_ASSERT(state != NULL);

	StateInfo info = state_info(state);
	TEST_ASSERT(info != NULL);

	TEST_ASSERT(info->current_portal == 0);
	TEST_ASSERT(info->wins == 0);
	TEST_ASSERT(info->playing);
	TEST_ASSERT(!info->paused);

	// Προσθέστε επιπλέον ελέγχους
}

void test_state_update() {
	State state = state_create();
	TEST_ASSERT(state != NULL && state_info(state) != NULL);

	// Πληροφορίες για τα πλήκτρα (αρχικά κανένα δεν είναι πατημένο)
	struct key_state keys = { false, false, false, false, false, false };
	
	// Χωρίς κανένα πλήκτρο, ο χαρακτήρας μετακινείται 7 pixels μπροστά
	Rectangle old_rect = state_info(state)->character->rect;
	state_update(state, &keys);
	Rectangle new_rect = state_info(state)->character->rect;

	TEST_ASSERT( new_rect.x == old_rect.x + 7 && new_rect.y == old_rect.y );

	// Με πατημένο το δεξί βέλος, ο χαρακτήρας μετακινείται 12 pixes μπροστά
	keys.right = true;
	old_rect = state_info(state)->character->rect;
	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;

	TEST_ASSERT( new_rect.x == old_rect.x + 12 && new_rect.y == old_rect.y );

	// Προσθέστε επιπλέον ελέγχους
}

 void test_state_objectsI(){
	State state = state_create();
	List list = state_objects(state, 1, 700);
	TEST_ASSERT(list_size(list) == 1);
 }

void test_state_objectsII(){
	State state = state_create();
	List list = state_objects(state, 1, 1400);
	TEST_ASSERT(list_size(list) == 2);
 }

void test_set_utils(){
	Set set = set_create( compare_ints, NULL );

	for( int i = 1; i <= 10; i++ ){
		set_insert( set, create_inT(i) );
	}

	int num = 8;

	Pointer a = set_find_eq_or_smaller( set, &num );
	Pointer b = set_find_eq_or_greater( set, &num );

	TEST_ASSERT( *a == 8 );
	TEST_ASSERT( *b == 8 );
	}

// Λίστα με όλα τα tests προς εκτέλεση
TEST_LIST = {
	{ "test_state_create", test_state_create },
	// { "test_state_update", test_state_update },
	{ "test_state_objectsI", test_state_objectsI},
	{ "test_state_objectsII", test_state_objectsII},
	//{ "test_set_utils", test_set_utils},

	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
};