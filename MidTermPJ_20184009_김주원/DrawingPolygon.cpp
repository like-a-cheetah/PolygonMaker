/*
		프로그램 명 : 폴리곤 컨트롤 프로그램
		작성일 : 2020.04.18 ~ 04.27
		작성자 : 컴퓨터소프트웨어공학과 20184009 김주원
		프로그램 설명 : Interactive하게 폴리곤을 그리는 프로그램,
					   마우스의 좌클릭으로 정점을 찍고, 마지막 정점과
					   마우스 커서에 항상 rubber band line이 연결된다.
					   우클릭시에 생성한 정점들이 연결되어 폴리곤이 그려진다.
					   그려진 폴리곤을 드래그하면 폴리곤이 마우스를 따라다니며
					   회전하게 되고, 중앙 버튼을 누를 경우, 그려진 모든것이
					   초기화되며 다시 그릴수 있게 된다.
*/
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>

GLfloat spin = 0.0;
float width = 500;
float height = 500;		//행렬의 각도와 윈도우의 크기

GLint drag_minY, drag_maxY, drag_minX, drag_maxX;	//드래그 창의 범위
GLint SPx, SPy, EPx, EPy;	//드래그의 시작점과 마지막점

float centerX = 0, centerY = 0;		//폴리곤의 중점
GLint polygon_minY, polygon_minX, polygon_maxY, polygon_maxX;	//폴리곤의 크기

int polygon = 0;			//폴리곤의 유무 판단 변수
int node_count = 0;			//정점의 개수 변수
int drag_activate = 0;		//드래그 활성 토글 변수
int selected_polygon = 0;	//폴리곤 드래그 판단 변수

typedef struct  Node {		//정점의 구조체 정의
	struct Node* link;
	float vertex[2];
}node;

node* vertex_list = NULL;	//정점 리스트 변수 선언

node* addVertex(float x, float y);
void remove_list();					//연결리스트 관련 함수들

void init();						//윈도우 관련 함수들
void reshape(int newWidth, int newHeight);
void display();

void mouseProcess(int button, int state, int x, int y);
void mouse_move(GLint x, GLint y);
void mouse_moveP(GLint x, GLint y);	//마우스의 동작 관련 함수들

void MoveObject();
void spin_Display();				//도형 움직임 관련 함수들

void draw_points();
void draw_lines();
void draw_drag();
void draw_polygons();
void point_numbers();				//윈도우에 그릴 그림 관련 함수들


int main(int argc, char** argv) {
	glutInit(&argc, argv);		//GLUT초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);	// 더블 버퍼와 RGB 색상 모드 설정
	glutInitWindowSize(width, height);		//윈도우 사이즈를 설정
	glutInitWindowPosition(150, 150);		//윈도우 생성 위치 설정
	glutCreateWindow("Interactive Drawing Polygon");		//윈도우를 생성한다
	init();						//윈도우창을 초기화한다

	glutDisplayFunc(display);		//display함수를 디스플레이 이벤트 콜백 함수로 등록
	glutReshapeFunc(reshape);		//변경된 렌더링 크기로 윈도우 크기 재설정
	glutMouseFunc(mouseProcess);	//mousePrcess함수를 마우스 이벤트 콜백 함수로 등록
	glutPassiveMotionFunc(mouse_moveP);		//윈도우 위에서 움직이는 마우스의 움직임 처리 함수
	glutMotionFunc(mouse_move);		//마우스의 클릭상태의 움직임 처리 함수 
	glutMainLoop();		//이벤트 루프로 진입
	remove_list();
	free(vertex_list);
	return 0;
}

void init() {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	//배경 색상 설정
	glMatrixMode(GL_PROJECTION);		//투영 행렬 모드로 변환
	glLoadIdentity();		//행렬을 단위 행렬로 만듦
	gluOrtho2D(0.0, width, height, 0.0);	//행렬을 볼 영역을 선택
}

void reshape(int newWidth, int newHeight) {	//윈도우 크기 만큼 렌더링 화면 비율을 바꾸기 위한 함수
	glViewport(0, 0, newWidth, newHeight);		//랜더링 영역 설정
	glMatrixMode(GL_PROJECTION);	//모델뷰 행렬을 가져온다
	glLoadIdentity();	//단위행렬로 초기화
	gluOrtho2D(0, newWidth, newHeight, 0);	//변경된 윈도우의 크기로 볼 행렬 설정
	glutPostRedisplay();	//바뀐 윈도우 재생(새로고침)
}

void display() {	//Display콜백 함수
	glClear(GL_COLOR_BUFFER_BIT);		//저장된 색으로 화면을 초기화
	glMatrixMode(GL_MODELVIEW);			//모델뷰 행렬을 가져옴
	glLoadIdentity();					//바뀐 윈도우를 재생시킴(새로고침)

	if (selected_polygon == 1) {	//폴리곤이 드래그로 선택된 경우,
		glutIdleFunc(spin_Display);	//각도를 계속 변경
		MoveObject();		//변경된 각도 상태로 특정 위치로 이동
	}

	//드래그 창 출력, 폴리곤 미생성시, 우클릭시, 폴리곤 선택시 출력x
	if (polygon == 1 && drag_activate == 1 && selected_polygon == 0)
		draw_drag();

	glColor3f(0, 0, 1);
	//폴리곤 출력, 폴리곤 미생성시 출력x
	if (polygon == 1)
		draw_polygons();

	glColor3f(1, 1, 1);
	draw_points();		//저장된 정점의 위치에 점 출력

	//선 출력, 정점 미생성시, 폴리곤 생성시 출력x
	if (vertex_list != NULL && polygon == 0)
		draw_lines();

	point_numbers();	//점 위에 숫자 출력

	glutSwapBuffers();	//버퍼를 교체하여, 그려진 것들 출력
}

void mouseProcess(int button, int state, int x, int y) {	//마우스 동작 처리 함수
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {	//왼쪽 버튼 클릭시
			if (polygon == 1)		//폴리곤 생성시,
				drag_activate = 1;		//드래그 활성화
			SPx = x, SPy = y;		//드래그의 시작점
			if (polygon == 0) {		//폴리곤 미생성시,
				vertex_list = addVertex(x, y);	//새로운 정점 리스트에 삽입
			}
			glutPostRedisplay();	//바뀐 윈도우 재생(새로고침)
			break;
		}
		else if (state == GLUT_UP) {	//클릭후 올라왔을 때
			if (drag_minX <= polygon_minX && drag_maxX >= polygon_maxX &&
				drag_minY <= polygon_minY && drag_maxY >= polygon_maxY)
				selected_polygon = 1;	//드래그 범위내 폴리곤 속할시, 선택 상태로 변환
			drag_activate = 0;		//드래그 비활성화
			break;
		}
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN) {	//오른쪽 버튼 클릭시
			if (polygon == 0) {		//폴리곤 미생성시
				polygon = 1;		//폴리곤 생성 표시
				vertex_list = addVertex(x, y);	//마지막 정점 좌표 리스트에 삽입
				centerX /= node_count, centerY /= node_count;	//정점들의 중점 계산
			}
			glutPostRedisplay();	//바뀐 윈도우 재생(새로고침)
			break;
		}
	case GLUT_MIDDLE_BUTTON:
		if (state == GLUT_DOWN) {		//가운데 버튼 클릭시, 초기화
			polygon = 0;				//폴리곤이 미생성된 상태
			node_count = 0;				//정점 개수 0
			selected_polygon = 0;		//선택된 폴리곤이 없는 상태
			drag_activate = 0;			//드래그 비활성화
			centerX = 0, centerY = 0;	//중점 0
			//남아있을 드래그 범위 초기화
			drag_minY = 0, drag_maxY = 0, drag_minX = 0, drag_maxX = 0;
			remove_list();				//정점 리스트 삭제

			spin = 0.0;					//각도 초기화
			glutIdleFunc(NULL);			//계속 동작하는 idle콜백 함수 정지

			init();						//윈도우 초기화
			glutPostRedisplay();		//바뀐 윈도우 재생(새로고침)
			break;
		}
	}
}

void mouse_move(GLint x, GLint y) {		//마우스 클릭후 이동함수
	EPx = x; EPy = y;
	glutPostRedisplay();	//바뀐 윈도우 재생(새로고침)
}

void mouse_moveP(GLint x, GLint y) {	//마우스 미클릭 이동함수
	EPx = x; EPy = y;
	glutPostRedisplay();	//바뀐 윈도우 재생(새로고침)
}

void MoveObject() {		//폴리곤의 중점을 마우스로 이동, 회전 함수
	glTranslatef(EPx, EPy, 0.0);	//3.회전된 오브젝트 마우스 위치로 이동
	glRotatef(spin, 0.0, 0.0, 1.0);	//2.좌표계의 원점에서 spin각도만큼 회전
	glTranslatef(-centerX, -centerY, 0.0);	//1.폴리곤 중점 좌표계 원점으로 이동
}

void spin_Display() {		//각도 변환 함수
	spin = spin + 2.5f;		//0.05씩 각도 변환
	if (spin > 360)			//한바퀴를 돌았다면 
		spin = spin - 360;	//다시 0도부터 회전
	glutPostRedisplay();	//바뀐 윈도우 재생(새로고침)
}

void draw_points() {	//정점 출력 함수
	node* vertex_head = NULL;
	glPointSize(4);
	glBegin(GL_POINTS);
	for (vertex_head = vertex_list; vertex_head != NULL; vertex_head = vertex_head->link) {
		glVertex2fv(vertex_head->vertex);	//리스트에 저장된 좌표에 점 출력
	}
	glEnd();
}

void draw_lines() {		//마지막 정점과 마우스 커서 러버밴드 연결
	glBegin(GL_LINES);
	glVertex2fv(vertex_list->vertex);
	glVertex2f(EPx, EPy);		//가장 마지막 정점과 마우스 좌표 연결선 출력
	glEnd();
}

void draw_polygons() {	//저장된 정점으로 폴리곤 출력
	node* vertex_head = NULL;
	glBegin(GL_POLYGON);
	for (vertex_head = vertex_list; vertex_head != NULL; vertex_head = vertex_head->link)
		glVertex2fv(vertex_head->vertex);
	glEnd();
}

void point_numbers() {
	node* vertex_head = NULL;
	int i = node_count, n = 0;
	char msg1[3];	//세자리 수까지 저장 가능
	glColor3f(1, 1, 1);
	if (vertex_list != NULL) {
		for (vertex_head = vertex_list; vertex_head != NULL; vertex_head = vertex_head->link) {
			i--;	//노드가 5개일 경우 5, 4, 3, 2, 1, 0의 순서로 출력한다.
			glRasterPos2fv(vertex_head->vertex);	//출력할 문자의 위치를 정함
			sprintf_s(msg1, "%d", i);	//정수를 문자로 변환
			for (n = 0; n <= i / 10; n++)	//자릿수를 계산하여 자릿수만큼 출력
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, msg1[n]);
		}
	}
}

void draw_drag() {
	//마우스의 시작점과 끝점의 좌표로 드래그창 생성

	glColor3f(0, 0, 0);
	glBegin(GL_QUADS);
	glVertex2f(SPx, SPy);
	glVertex2f(SPx, EPy);
	glVertex2f(EPx, EPy);
	glVertex2f(EPx, SPy);
	glEnd();

	glColor3f(1, 1, 1);
	glBegin(GL_LINES);
	glVertex2f(SPx, SPy);
	glVertex2f(SPx, EPy);

	glVertex2f(SPx, EPy);
	glVertex2f(EPx, EPy);

	glVertex2f(EPx, EPy);
	glVertex2f(EPx, SPy);

	glVertex2f(EPx, SPy);
	glVertex2f(SPx, SPy);
	glEnd();

	if (SPx > EPx) {
		drag_maxX = SPx;
		drag_minX = EPx;
	}
	else {
		drag_maxX = EPx;
		drag_minX = SPx;
	}

	if (SPy > EPy) {
		drag_maxY = SPy;
		drag_minY = EPy;
	}
	else {
		drag_maxY = EPy;
		drag_minY = SPy;
	}//시작점과 끝점의 좌표로 드래그 범위 도출
}

node* addVertex(float x, float y) {	//리스트의 맨 처음에 노드를 추가하는 함수
	node* newNode = (node*)malloc(sizeof(node));
	newNode->vertex[0] = x;		//x축 값
	newNode->vertex[1] = y;		//y축 값
	newNode->link = vertex_list;
	//추가되는 좌표값을 폴리곤 중점 변수에 합함
	centerX += x; centerY += y;
	if (vertex_list == NULL) {	//첫 정점으로 폴리곤의 범위 초기화
		polygon_minY = y, polygon_minX = x, polygon_maxY = y, polygon_maxX = x;
	}
	else {	//추가되는 정점이 기존의 범위를 넘어가면 다시 초기화
		if (x > polygon_maxX)
			polygon_maxX = x;
		else if (x < polygon_minX)
			polygon_minX = x;

		if (y > polygon_maxY)
			polygon_maxY = y;
		else if (y < polygon_minY)
			polygon_minY = y;
	}
	vertex_list = newNode;
	node_count++;		//노드 개수 +1
	return vertex_list;	//정점이 추가된 리스트 반환
}

void remove_list() {	//연결리스트 삭제 함수
	node* temp = NULL;
	while (vertex_list != NULL) {
		temp = vertex_list->link;
		free(vertex_list);
		vertex_list = temp;
	}
}