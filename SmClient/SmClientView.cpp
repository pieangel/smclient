
// SmClientView.cpp : implementation of the CSmClientView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "SmClient.h"
#endif

#include "SmClientDoc.h"
#include "SmClientView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSmClientView

IMPLEMENT_DYNCREATE(CSmClientView, CView)

BEGIN_MESSAGE_MAP(CSmClientView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CSmClientView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CSmClientView construction/destruction

CSmClientView::CSmClientView() noexcept
{
	// TODO: add construction code here

}

CSmClientView::~CSmClientView()
{
}

BOOL CSmClientView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CSmClientView drawing

void CSmClientView::OnDraw(CDC* /*pDC*/)
{
	CSmClientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CSmClientView printing


void CSmClientView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CSmClientView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CSmClientView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CSmClientView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CSmClientView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CSmClientView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CSmClientView diagnostics

#ifdef _DEBUG
void CSmClientView::AssertValid() const
{
	CView::AssertValid();
}

void CSmClientView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSmClientDoc* CSmClientView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSmClientDoc)));
	return (CSmClientDoc*)m_pDocument;
}
#endif //_DEBUG


// CSmClientView message handlers
