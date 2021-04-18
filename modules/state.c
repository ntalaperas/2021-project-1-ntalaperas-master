
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "ADTVector.h"
#include "ADTList.h"
#include "state.h"

// Οι ολοκληρωμένες πληροφορίες της κατάστασης του παιχνιδιού.
// Ο τύπος State είναι pointer σε αυτό το struct, αλλά το ίδιο το struct
// δεν είναι ορατό στον χρήστη.

struct state {
	Vector objects;			// περιέχει στοιχεία Object (Εμπόδια / Εχθροί / Πύλες)
	List portal_pairs;		// περιέχει PortaPair (ζευγάρια πυλών, είσοδος/έξοδος)

	struct state_info info;
};

// Ζευγάρια πυλών

typedef struct portal_pair {
	Object entrance;		// η πύλη entrance
	Object exit;			// οδηγεί στην exit
}* PortalPair;

int compare_portal_pairs_entrances(Pointer a, Pointer b) {
	return ((PortalPair)a)->entrance == ((PortalPair)b)->entrance ? 0:1;
}

int compare_portal_pairs_exits(Pointer a, Pointer b) {
	return ((PortalPair)a)->exit == ((PortalPair)b)->exit ? 0:1;
}

// Δεσμεύει και επιστρέφει έναν ακέραιο στο σωρό

int* create_int(int value){
	int* p = malloc(sizeof(int));
	*p = value;
	return p;
}

// Δημιουργεί και επιστρέφει την αρχική κατάσταση του παιχνιδιού

State state_create() {
	// Δημιουργία του state
	State state = malloc(sizeof(*state));

	// Γενικές πληροφορίες
	state->info.current_portal = 0;			// Δεν έχουμε περάσει καμία πύλη
	state->info.wins = 0;					// Δεν έχουμε νίκες ακόμα
	state->info.playing = true;				// Το παιχνίδι ξεκινάει αμέσως
	state->info.paused = false;				// Χωρίς να είναι paused.

	// Πληροφορίες για το χαρακτήρα.
	Object character = state->info.character = malloc(sizeof(*character));
	character->type = CHARACTER;
	character->forward = true;
	character->jumping = false;

    // Ο χαρακτήρας (όπως και όλα τα αντικείμενα) έχουν συντεταγμένες x,y σε ένα
    // καρτεσιανό επίπεδο.
	// - Στο άξονα x το 0 είναι η αρχή στης πίστας και οι συντεταγμένες
	//   μεγαλώνουν προς τα δεξιά.
	// - Στον άξονα y το 0 είναι το "δάπεδο" της πίστας, και οι
	//   συντεταγμένες μεγαλώνουν προς τα _κάτω_.
	// Πέρα από τις συντεταγμένες, αποθηκεύουμε και τις διαστάσεις width,height
	// κάθε αντικειμένου. Τα x,y,width,height ορίζουν ένα παραλληλόγραμμο, οπότε
	// μπορούν να αποθηκευτούν όλα μαζί στο obj->rect τύπου Rectangle (ορίζεται
	// στο include/raylib.h).
	// 
	// Προσοχή: τα x,y αναφέρονται στην πάνω-αριστερά γωνία του Rectangle, και
	// τα y μεγαλώνουν προς τα κάτω, οπότε πχ ο χαρακτήρας που έχει height=38,
	// αν θέλουμε να "κάθεται" πάνω στο δάπεδο, θα πρέπει να έχει y=-38.

	character->rect.width = 70;
	character->rect.height = 38;
	character->rect.x = 0;
	character->rect.y = - character->rect.height;

	// Δημιουργία των objects (πύλες / εμπόδια / εχθροί) και προσθήκη στο vector
	// state->objects. Η πίστα περιέχει συνολικά 4*PORTAL_NUM αντικείμενα, από
	// τα οποία τα PORTAL_NUM είναι πύλες, και τα υπόλοια εμπόδια και εχθροί.

	state->objects = vector_create(0, free);		// Δημιουργία του vector

	for (int i = 0; i < 4*PORTAL_NUM; i++) {
		// Δημιουργία του Object και προσθήκη στο vector
		Object obj = malloc(sizeof(*obj));
		vector_insert_last(state->objects, obj);

		// Κάθε 4 αντικείμενα υπάρχει μια πύλη. Τα υπόλοιπα αντικείμενα
		// επιλέγονται τυχαία.

		if(i % 4 == 3) {							// Το 4ο, 8ο, 12ο κλπ αντικείμενο
			obj->type = PORTAL;						// είναι πύλη.
			obj->rect.width = 100;
			obj->rect.height = 5;

		} else if(rand() % 2 == 0) {				// Για τα υπόλοιπα, με πιθανότητα 50%
			obj->type = OBSTACLE;					// επιλέγουμε εμπόδιο.
			obj->rect.width = 10;
			obj->rect.height = 80;

		} else {
			obj->type = ENEMY;						// Και τα υπόλοιπα είναι εχθροί.
			obj->rect.width = 30;
			obj->rect.height = 30;
			obj->forward = false;					// Οι εχθροί αρχικά κινούνται προς τα αριστερά.
		}

		// Τα αντικείμενα είναι ομοιόμορφα τοποθετημένα σε απόσταση SPACING
		// μεταξύ τους, και "κάθονται" πάνω στο δάπεδο.

		obj->rect.x = (i+1) * SPACING;
		obj->rect.y = - obj->rect.height;
	}

	// TODO: αρχικοποίηση της λίστας obj->portal_pairs
	state->portal_pairs = list_create(free);

	srand(time(NULL));

	for (int i = 3; i < 4*PORTAL_NUM; i+=4) {			//περνάω τα entrances απο τα PORTAL στη λίστα state->portal_pairs
		PortalPair pp = malloc(sizeof(PortalPair));
		pp->entrance = vector_get_at(state->objects, i);
		pp->exit = NULL;
	}
	for (ListNode lnode = list_first(state->portal_pairs);
		lnode != LIST_EOF;
		lnode = list_next(state->portal_pairs, lnode)) {

		bool inserted = false;

		PortalPair pp = malloc(sizeof(PortalPair));
		int pos = (rand()/100+1)*4-1;					//3-399 ανα 4
		pp->exit = vector_get_at(state->objects, pos);		
		while (inserted != true) {							//οσο δεν εχει καθοριστεί το exit
			if (list_find_node(state->portal_pairs, pp, compare_portal_pairs_exits) == LIST_EOF &&
				((PortalPair)(list_node_value(state->portal_pairs, lnode)))->entrance != pp->exit) {		//αν δεν υπάρχει το exit ηδη βαλτο
					((PortalPair)(list_node_value(state->portal_pairs, lnode)))->exit = pp->exit;
					inserted = true;			
				}
				else {
					pos +=4;
					if (pos == 403)		// αν ξεπεράσω το όριο των portals
						pos = 3;
					pp->exit = vector_get_at(state->objects, pos);
				}
			}
		}

	return state;
}

// Επιστρέφει τις βασικές πληροφορίες του παιχνιδιού στην κατάσταση state

StateInfo state_info(State state) {
	if (state != NULL)			// Αν έχει ξεκινήσει το παιχνίδι επιστρέφει τις βασικές πληροφορίες
		return &(state->info);
	return NULL;
}

// Επιστρέφει μια λίστα με όλα τα αντικείμενα του παιχνιδιού στην κατάσταση state,
// των οποίων η συντεταγμένη x είναι ανάμεσα στο x_from και x_to.

List state_objects(State state, float x_from, float x_to) {
	if (state != NULL && state->objects != NULL){	

		List obj = list_create(free);

		for (VectorNode node = vector_first(state->objects);
			node != VECTOR_EOF;
			node = vector_next(state->objects, node)){

				Object obj1 = vector_node_value(state->objects, node);
				float x = obj1->rect.x;
				if (x >= x_from && x <= x_to)
					list_insert_next(obj, LIST_EOF, obj1);		//βαζω στην καινουρια μου λιστα μονο τα στοιχεια που καλυπτουν τις προυποθεσεις
		}

		return obj;
	}else

		return NULL;
}

// Ενημερώνει την κατάσταση state του παιχνιδιού μετά την πάροδο 1 frame.
// Το keys περιέχει τα πλήκτρα τα οποία ήταν πατημένα κατά το frame αυτό.

void state_update(State state, KeyState keys) {

	if( state->info.playing == false ){
		if( keys->enter == true ){
			state->info.playing = true;
		}
	}else if( state->info.playing == true ){
		if( keys->p == true && state->info.paused == false ){
			state->info.paused = true;
		}
	}else if( state->info.paused == true ){
		if( keys->n == true ){
			state_update(state, keys);
		}
	}else {
		Object nchar = malloc(sizeof(Object));

		if( nchar->type == ENEMY ){
			List list = state_objects(state, nchar->rect.x-1, nchar->rect.x+1);

			if ( list == NULL ){
				if( nchar->forward == true ){
					nchar->rect.x += 5;
				}else{
					nchar->rect.x -= 5;
				}
			}else{
				Object type = list_node_value( list, list_first(list) );

				if( type->type == OBSTACLE ){
					if ( CheckCollisionRecs( type->rect, nchar->rect )){
						if( nchar->forward == true ){
							nchar->forward = false;
						}else{
							nchar->forward = true;
						}
					}
				}else if( type->type == PORTAL ){
					if ( CheckCollisionRecs( type->rect, nchar->rect )){
							if( nchar->forward == false ){
								PortalPair pair = list_find( state->portal_pairs, type, compare_portal_pairs_exits );
								nchar->rect.x = pair->entrance->rect.x - nchar->rect.width;							
							}else{
								PortalPair pair = list_find( state->portal_pairs, type, compare_portal_pairs_entrances );
								nchar->rect.x = pair->exit->rect.x + nchar->rect.width;
							}
					}
				}


			}	
		}
		else if( nchar->type == CHARACTER ){
			List list = state_objects(state, nchar->rect.x-1, nchar->rect.x+1);

			if ( list == NULL ){
				if( nchar->forward == true ){
					if( keys->right == true ){
						nchar->rect.x += 12;
					}else if( keys->right == false && keys->left == false ){
						nchar->rect.x += 7;
					}else if( keys->right == false && keys->left == true ){
						nchar->forward = false;
					}
				}else{
					if( keys->left == true ){
						nchar->rect.x -= 12;
					}else if( keys->left == false && keys->right == false ){
						nchar->rect.x -= 7;
					}else if( keys->left == false && keys->right == true ){
						nchar->forward = true;
					}
				}
				
				if( nchar->jumping == true ){
					if( keys->up == true && nchar->rect.y -38 > -220 ){
						nchar->rect.y -= 15;
					}else if( keys->up == true && nchar->rect.y -38 <= -220 ){
						nchar->jumping = false;
					}
				}else{
					if( keys->up == false && nchar->rect.y +38 < 0 ){
						nchar->rect.y += 15;
					}else if( keys->up == false && nchar->rect.y + 38 >= 0 ){
						nchar->rect.y = -38;
					}else if( keys->up == true ){
						nchar->jumping = true;
					}
				}
			}else {
				Object type = list_node_value( list, list_first(list) );

				if( type->type == OBSTACLE || type->type == ENEMY ){
					if( CheckCollisionRecs( type->rect, nchar->rect) ){
						state->info.playing = false;
					}
				}else if( type->type == PORTAL ){
					Object finalgate = vector_get_at( state->objects, 399 );
					if ( CheckCollisionRecs( finalgate->rect, nchar->rect) ){
						state->info.wins += 1;
						nchar->rect.x = 0;
					}else if ( CheckCollisionRecs( type->rect, nchar->rect ) ){
							if( nchar->forward == false ){
								PortalPair pair = list_find( state->portal_pairs, type, compare_portal_pairs_exits );
								nchar->rect.x = pair->entrance->rect.x - nchar->rect.width;							
							}else{
								PortalPair pair = list_find( state->portal_pairs, type, compare_portal_pairs_entrances );
								nchar->rect.x = pair->exit->rect.x + nchar->rect.width;
							}
					}
					
				}

			}


		}
	}
}

// Καταστρέφει την κατάσταση state ελευθερώνοντας τη δεσμευμένη μνήμη.

void state_destroy(State state) {
	list_destroy(state->portal_pairs);
	vector_destroy(state->objects);
}