/*
 *	minesweeper.c
 *	ig
 *
 *	Created by Denis Becker on 12.02.16.
 *	Copyright 2016 Ensimag. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h> // for random seed
#include <string.h> // memset function

#include "ei_application.h"
#include "ei_widget.h"
#include "ei_utils.h"
#include "ei_event.h"

/* constants */

const int			k_info_height	= 40;
ei_size_t			g_button_size	= { 28, 28 };
ei_color_t			g_bg_color	= {0x88, 0x88, 0x88, 0xff};

/* global resources */

static ei_surface_t		g_flag_img;
static ei_surface_t		g_bomb_img;
static ei_surface_t		g_reset_img;

/* structs & typedefs */

struct map_t;

typedef struct {

	struct map_t*		map;
	ei_point_t		coord;
	ei_bool_t		has_mine;
	ei_bool_t		has_flag;
	ei_bool_t		is_revealed;
	int			nb_mines_around;
	ei_widget_t*		widget;
	
} map_cell_t;

typedef struct map_t {

	int			width;
	int			height;
	int			nb_mines;
	map_cell_t*		cells;
	ei_bool_t		game_over;
	int			nb_revealed;
	int			flag_count;
	double 			start_time;
	ei_widget_t*		toplevel;
	ei_widget_t*		flag_count_widget;
	ei_widget_t*		victory_text_widget;
	ei_widget_t*		time_widget;

} map_t;

/* utility functions */

static void create_cell_buttons(map_t* map);
void set_time(map_t *p_map, int time);

static inline map_cell_t* get_cell_at(map_t* map, int x, int y)
{
	return &map->cells[y * map->width + x];
}

static inline ei_bool_t check_bounds(map_t* map, int x, int y)
{
	return x >= 0 && y >= 0 && x < map->width && y < map->height;
}

static inline ei_bool_t can_play(map_cell_t* map_pos)
{
	return ((!map_pos->has_flag) && (!map_pos->is_revealed));
}

void int_to_str(int val, char *txt, int txtlen)
{
	ei_bool_t	positive	= (val >= 0);
	int		idx		= txtlen - 1;

	val = (positive ? val : -val);

	do {
		txt[idx] = '0' + (char) (val % 10);
		val /= 10;
		if (val == 0)
			break;
	} while (idx-- > 0);

	if (!positive && idx > 0)
		txt[idx - 1] = '-';
}

/* core functions */

/*
 * check_victory: configures the victory message if victory.
 */
static ei_bool_t check_victory(map_t* map)
{
	ei_bool_t	victory = (map->nb_revealed + map->nb_mines == map->width * map->height);
	char*		victory_text = "You won!";
	ei_color_t	victory_back = {0x33, 0x33, 0x33, 0xff};
	ei_color_t	victory_color = {0x55, 0xbb, 0x55, 0xff};

	if (victory) {
		map->game_over	= EI_TRUE;
		map->start_time	= -1.0;
		ei_frame_configure(map->victory_text_widget, NULL, &victory_back, NULL, NULL,
					&victory_text, NULL, &victory_color, NULL, NULL, NULL, NULL);
	}
	return victory;
}

/*
 * update the flag count widget.
 */
void update_flag_count(map_t* map)
{
	char		txt[5];
	memset(txt, ' ', 4);
	txt[4]		= '\0';
	char*		txtptr = (char*)&txt;

	int_to_str(map->flag_count, txt, 4);
	ei_frame_configure(map->flag_count_widget, NULL, NULL, NULL, NULL, &txtptr, NULL, NULL,
					NULL, NULL, NULL, NULL);
}

void switch_flag(map_cell_t* map_pos)
{
	map_pos->has_flag = !map_pos->has_flag;

	if (map_pos->has_flag) {
		ei_button_configure(map_pos->widget, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				   &g_flag_img, NULL, NULL, NULL, NULL);
		map_pos->map->flag_count--;
	} else {
		ei_button_configure(map_pos->widget, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				   &g_reset_img, NULL, NULL, NULL, NULL);
		map_pos->map->flag_count++;
	}
	update_flag_count(map_pos->map);
}

/*
 * Reveal one position on the map, don't explore neighbors, don't check
 * if already revealed (must be done by the caller), don't end game if it's a mine.
 */
void reveal_position_no_check(map_cell_t* cell)
{
	char		nb_txt[2]	= " ";
	char*		nb_txt_ptr	= (char*) nb_txt;
	ei_relief_t	relief		= ei_relief_none;
	ei_point_t	parent_pos;

	cell->is_revealed = EI_TRUE;
	cell->map->nb_revealed++;

	ei_widget_destroy(cell->widget);
	cell->widget = ei_widget_create("frame", cell->map->toplevel, (void*)cell, NULL);

	parent_pos.x = cell->coord.x * g_button_size.width;
	parent_pos.y = cell->coord.y * g_button_size.height + k_info_height;

	ei_place(cell->widget, NULL, &parent_pos.x, &parent_pos.y, NULL, NULL, NULL, NULL, NULL, NULL);

	if (cell->has_mine) {
		ei_frame_configure(cell->widget, &g_button_size, NULL, NULL, &relief, NULL, NULL,
				   			 NULL, NULL, &g_bomb_img, NULL, NULL);
	} else if (cell->nb_mines_around > 0) {
		int_to_str(cell->nb_mines_around, nb_txt, 1);
		ei_frame_configure(cell->widget, &g_button_size, NULL, NULL, NULL, &nb_txt_ptr,
				   			 NULL, NULL, NULL, NULL, NULL, NULL);
	} else
		ei_frame_configure(cell->widget, &g_button_size, NULL, NULL, NULL, NULL,
				   			 NULL, NULL, NULL, NULL, NULL, NULL);
}

void defeat(map_t* map)
{
	char*		defeat_text = "You lost!";
	ei_color_t	defeat_back = {0x33, 0x33, 0x33, 0xff};
	ei_color_t	defeat_color = {0xbb, 0x55, 0x55, 0xff};

	map->game_over	= EI_TRUE;
	map->start_time	= -1.0;

	for (int i = 0; i < map->width * map->height; i++)
		if (map->cells[i].has_mine)
			reveal_position_no_check(&map->cells[i]);
	
	ei_frame_configure(map->victory_text_widget, NULL, &defeat_back, NULL, NULL, &defeat_text, NULL, &defeat_color,
			NULL, NULL, NULL, NULL);
}

/*
 * Caller must ensure that map_cell isn't already revealed.
 */
void reveal_and_explore(map_cell_t* map_cell)
{
	int			x;
	int			y;
	int			cx	= map_cell->coord.x;
	int			cy	= map_cell->coord.y;
	map_t*			map	= map_cell->map;
	map_cell_t*		next;

	reveal_position_no_check(map_cell);
	if (map_cell->has_mine) {
		defeat(map);
		return;

	} else if (map_cell->nb_mines_around > 0)
		// explosion risk around, stop exploration
		return;

	for (y = cy - 1; y <= cy + 1; y++)
		for (x = cx - 1; x <= cx + 1; x++) {
			next = get_cell_at(map, x, y);
			if ((x != cx || y != cy) && check_bounds(map, x, y)
					&& (!next->is_revealed) && (!next->has_flag))
				reveal_and_explore(next);
		}
}

/*
 * set_mine_in_cell: places a mine in the given cell.
 */
void set_mine_in_cell(map_cell_t* map_cell)
{
	int	x;
	int	y;
	int	cx	= map_cell->coord.x;
	int	cy	= map_cell->coord.y;
	map_t*	map	= map_cell->map;

	map_cell->has_mine = EI_TRUE;

	// update neighbors mine count
	for (y = cy - 1; y <= cy + 1; y++)
		for (x = cx - 1; x <= cx + 1; x++)
			if ((x != cx || y != cy) && check_bounds(map, x, y))
				get_cell_at(map, x, y)->nb_mines_around++;
}

/*
 * place_mines: places the mines on an empty map.
 */
void place_mines(map_t* map)
{
	int i, n;

	for (n = 0; n < map->nb_mines; n++) {
		do {
			i = rand() % (map->width * map->height);
		} while (map->cells[i].has_mine);

		set_mine_in_cell(&map->cells[i]);
	}
}


/*
 * reset_game: creates a new grid.
 */
void reset_game(map_t* map)
{
	int		i;
	char*		nulltext	= NULL;
	ei_relief_t	relief		= ei_relief_raised;
	ei_color_t	color		= {0xbb, 0xbb, 0xbb, 0xff};
	map_cell_t*	cell;

	map->flag_count			= map->nb_mines;
	map->game_over			= EI_FALSE;
	map->nb_revealed		= 0;
	map->start_time			= -1.0;

	for (i = 0, cell = map->cells; i < map->width * map->height; i++, cell++) {
		cell->has_mine		= EI_FALSE;
		cell->has_flag		= EI_FALSE;
		cell->is_revealed	= EI_FALSE;
		cell->nb_mines_around	= 0;

		ei_widget_destroy(cell->widget);
	}
	create_cell_buttons(map);

	place_mines(map);
	update_flag_count(map);
	ei_frame_configure(map->victory_text_widget, NULL, &color, NULL, NULL, &nulltext, NULL, NULL,
								NULL, NULL, NULL, NULL);
}

/*
 * Create and fill the mine map.
 * 'map' must point to an allocated 'map_t' struct.
 */
void create_mine_map(map_t* map, int width, int height, int nb_mines)
{
	int		x, y;
	map_cell_t*	cell;
	
	map->width		= width;
	map->height		= height;
	map->nb_mines		= nb_mines;
	map->cells		= (map_cell_t*)calloc(width * height, sizeof(map_cell_t));
	map->game_over		= EI_FALSE;
	map->nb_revealed	= 0;
	map->flag_count		= nb_mines;
	map->start_time		= -1.0;

	for (y = 0, cell = map->cells; y < map->height; y++)
		for (x = 0; x < map->width; x++, cell++) {
			cell->map	= map;
			cell->coord	= ei_point(x, y);
		}

	place_mines(map);
}

void destroy_mine_map(map_t* map)
{
	free(map->cells);
}

/* event handlers */

void handle_time(map_t* map)
{
	static int	prev_int_time		= -1;
	int 		int_time;

	if (map->start_time == -1.0)
		return;

	int_time = (int)floor(hw_now() - map->start_time);
	if (int_time != prev_int_time) {
		set_time(map, int_time);
		prev_int_time	= int_time;
	}
}

void set_time(map_t* map, int time)
{
	char 	time_str[20];
	char*	time_str_ptr	= time_str;

	sprintf(time_str, "%d", time);
	ei_frame_configure(map->time_widget, NULL, NULL, NULL, NULL, &time_str_ptr, NULL, NULL, NULL, NULL, NULL, NULL);
}

ei_bool_t default_handler(ei_event_t* event)
{
	if ((event->type == ei_ev_keydown) && (event->param.key.key_code == SDLK_ESCAPE)) {
		ei_app_quit_request();
		return EI_TRUE;
	} else if (event->type == ei_ev_app) {
		handle_time((map_t*)(event->param.application.user_param));
		hw_event_schedule_app(250, event->param.application.user_param);
	}

	return EI_FALSE;
}

void handle_restart(ei_widget_t* widget, ei_event_t* event, void* user_param)
{
	map_t* map = (map_t*) user_param;
	reset_game(map);
}

void cell_button_handler(ei_widget_t* widget, ei_event_t* event, void* user_param)
{
	ei_relief_t		relief = ei_relief_none;

	map_cell_t*		cell = (map_cell_t*)(widget->user_data);

	if (cell->map->game_over)
		return;

	if (cell->map->start_time == -1.0)
		cell->map->start_time	= hw_now();

	if (event->param.mouse.button == ei_mouse_button_left) {
		if (can_play(cell)) {
			reveal_and_explore(cell);
			check_victory(cell->map);
			return;
		}

	} else if (event->param.mouse.button == ei_mouse_button_right) {
		if (!cell->is_revealed) {
			switch_flag(cell);
			return;
		}
	}
}

void create_cell_buttons(map_t* map)
{
	int 		x, y;
	int		border_width	= 2;
	int		corner_radius	= 0;
	ei_relief_t	relief		= ei_relief_raised;
	ei_callback_t 	callback	= cell_button_handler;
	map_cell_t*	cell;
	ei_widget_t*	button;
	ei_point_t	coord;

	for (y = 0, cell = map->cells; y < map->height; y++) {
		for (x = 0; x < map->width; x++, cell++) {

			button = ei_widget_create("button", map->toplevel, (void*)cell, NULL);
			cell->widget = button;
			ei_button_configure(button, &g_button_size, &g_bg_color, &border_width, &corner_radius, &relief, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &callback, NULL);

			coord.x = x * g_button_size.width;
			coord.y = k_info_height + y * g_button_size.height;
			ei_place(button, NULL, &(coord.x), &(coord.y), NULL, NULL, NULL, NULL, NULL, NULL);
		}
	}
}

/*
 * Create the game window, 'map' must have been created before (with 'create_mine_map').
 */
void create_game_window(map_t* map)
{
	char 		label_str[80];
	char*		label_str_ptr	= label_str;
	ei_size_t	size;
	ei_anchor_t	anchor		= ei_anc_north;

	ei_size_t	toplevel_size	= ei_size(g_button_size.width * map->width, k_info_height + g_button_size.height * map->height);
	ei_color_t	toplevel_bg	= {0xbb, 0xbb, 0xbb, 0xff};
	ei_axis_set_t	resizable	= ei_axis_none;
	int		border_width;
	int		corner_radius	= 0;
	ei_point_t 	coord;

	ei_widget_t*	widget;
	ei_color_t	light		= {0xcc, 0xcc, 0xcc, 0xcc};
	ei_callback_t	callback	= handle_restart;

	char		text[5];

	memset(text, ' ', 4);
	text[4]			= '\0';
	char* textptr		= (char*) &text;

	// toplevel

	map->toplevel		= ei_widget_create("toplevel", ei_app_root_widget(), NULL, NULL);
	coord.x = 40;
	coord.y = 40;
	border_width = 0;
	strcpy(label_str, "Minesweeper");
	ei_toplevel_configure(map->toplevel, &toplevel_size, &toplevel_bg, &border_width, &label_str_ptr, NULL, &resizable, NULL);
	ei_place(map->toplevel, NULL, &(coord.x), &(coord.y), NULL, NULL, NULL, NULL, NULL, NULL);

	// restart button

	coord.x = toplevel_size.width / 2;
	coord.y = 4;
	size = ei_size(100, k_info_height - 2 * coord.y);
	border_width = 2;
	widget = ei_widget_create("button", map->toplevel, NULL, NULL);
	strcpy(label_str, "Restart");
	ei_button_configure(widget, &size, &g_bg_color, &border_width, &corner_radius, NULL, &label_str_ptr, NULL, NULL,
			NULL, NULL, NULL, NULL, &callback, (void*) &map);
	ei_place(widget, &anchor, &(coord.x), &(coord.y), NULL, NULL, NULL, NULL, NULL, NULL);

	// flag count label

	coord.x = 4;
	size.width = 70;
	anchor = ei_anc_northwest;
	int_to_str(map->nb_mines, text, 4);
	widget = ei_widget_create("frame", map->toplevel, NULL, NULL);
	ei_frame_configure(widget, &size, &light, &border_width, NULL, &textptr, NULL, NULL,
			NULL, NULL, NULL, NULL);
	ei_place(widget, &anchor, &(coord.x), &(coord.y), NULL, NULL, NULL, NULL, NULL, NULL);
	map->flag_count_widget = widget;

	// timer

	coord.x = 80;
	size.width = 70;
	anchor = ei_anc_northwest;
	map->time_widget = ei_widget_create("frame", map->toplevel, NULL, NULL);
	char time_str[20];
	char* time_str_ptr = time_str;
	sprintf(time_str, "%d", 0);
	ei_frame_configure(map->time_widget, &size, &light, &border_width, NULL, &time_str_ptr, NULL, NULL,
			NULL, NULL, NULL, NULL);
	ei_place(map->time_widget, &anchor, &(coord.x), &(coord.y), NULL, NULL, NULL, NULL, NULL, NULL);

	// game result text

	coord.x = toplevel_size.width - 4;
	size.width = 90;
	anchor = ei_anc_northeast;
	widget = ei_widget_create("frame", map->toplevel, NULL, NULL);
	ei_frame_configure(widget, &size, &toplevel_bg, &border_width, NULL, NULL, NULL, NULL,
			NULL, NULL, NULL, NULL);
	ei_place(widget, &anchor, &(coord.x), &(coord.y), NULL, NULL, NULL, NULL, NULL, NULL);
	map->victory_text_widget = widget;

	create_cell_buttons(map);
}

int get_arg_value(char *arg, int min, int max, int def)
{
	int try = atoi(arg);
	if (try < min || try > max) {
		return def;
	}
	return try;
}



// main -

int main(int argc, char* argv[])
{
	srand((unsigned)time(NULL)); // random seed

	const int	w_min = 12, w_max = 32;
	const int	h_min = 4, h_max = 22;
	int		size_w = 20;
	int		size_h = 16;
	int		nb_mines = 40;

	if (argc == 4) {
		size_w = get_arg_value(argv[1], w_min, w_max, size_w);
		size_h = get_arg_value(argv[2], h_min, h_max, size_h);
		nb_mines = get_arg_value(argv[3], 1, size_w * size_h, nb_mines);
	} else if (argc > 1) {
		printf("Using default values\n");
		printf("Usage: minesweeper WIDTH HEIGHT NB_MINES\n	WIDTH range = (%d, %d) def. %d\n", w_min, w_max, size_w);
		printf("  HEIGHT range = (%d, %d) def. %d\n	 NB_MINES > 0 and < WIDTH * HEIGHT def. %d\n", h_min, h_max, size_h, nb_mines);
	}

	map_t		map;
	ei_size_t	root_window_size		= { 1024, 768 };
	ei_color_t	root_bgcol			= { 0x52, 0x7f, 0xb4, 0xff };
	ei_bool_t	fullscreen			= EI_FALSE;

	ei_app_create(root_window_size, fullscreen);
	ei_frame_configure(ei_app_root_widget(), NULL, &root_bgcol, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	if ((g_flag_img = hw_image_load("misc/flag.png", ei_app_root_surface())) == NULL) {
		printf("ERROR: could not load image \"misc/flag.png\"");
		return 1;
	}
	if ((g_bomb_img = hw_image_load("misc/bomb.png", ei_app_root_surface())) == NULL) {
		printf("ERROR: could not load image \"misc/bomb.png\"");
		return 1;
	}
	g_reset_img = NULL;

	create_mine_map(&map, size_w, size_h, nb_mines);
	create_game_window(&map);

	ei_event_set_default_handle_func(&default_handler);

	hw_event_schedule_app(250, &map);
	ei_app_run();

	destroy_mine_map(&map);

	hw_surface_free(g_flag_img);
	hw_surface_free(g_bomb_img);

	ei_app_free();

	return (EXIT_SUCCESS);
}
