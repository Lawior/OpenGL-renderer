#pragma once

void renderer_camera_move_delta_right(double delta_time);
void renderer_camera_move_delta_left(double delta_time);
void renderer_camera_move_delta_up(double delta_time);
void renderer_camera_move_delta_down(double delta_time);
void renderer_camera_move_delta_forward(double delta_time);
void renderer_camera_move_delta_back(double delta_time);

void renderer_camera_rotate(int x_rel, int y_rel);

void renderer_init(int w, int h);
void renderer_resize(int w, int h);
void renderer_render();
void renderer_end();

