/* uninit.c: Implementation of uninitialized page.

All of the pages are born as uninit page. When the first page fault occurs,
the handler chain calls uninit_initialize (page->operations.swap_in).
The uninit_initialize function transmutes the page into the specific page
object (anon, file, page_cache), by initializing the page object,and calls
initialization callback that passed from vm_alloc_page_with_initializer
function.

uninit.c: 초기화되지 않은 페이지(uninit page)의 구현입니다.


모든 페이지는 초기에 uninit page로 생성됩니다. 첫 번째 페이지 폴트가 발생하면,
핸들러 체인이 uninit_initialize(page->operations.swap_in)을 호출합니다.
uninit_initialize 함수는 페이지를 특정한 페이지 객체(익명 페이지, 파일 페이지, 페이지 캐시)로 변환합니다.
이를 위해 페이지 객체를 초기화하고, vm_alloc_page_with_initializer 함수로부터 전달된 초기화 콜백을 호출합니다.*/

#include "vm/vm.h"
#include "vm/uninit.h"


/* DO NOT MODIFY this struct 이 구조체를 수정하지 마십시오.  */
static const struct page_operations uninit_ops = {
	.swap_in = uninit_initialize,
	.swap_out = NULL,
	.destroy = uninit_destroy,
	.type = VM_UNINIT,
};

/* DO NOT MODIFY this function 이 함수를 수정하지 마십시오. */
void
uninit_new (struct page *page, void *va, vm_initializer *init,
		enum vm_type type, void *aux,
		bool (*initializer)(struct page *, enum vm_type, void *)) {
	ASSERT (page != NULL);

	*page = (struct page) {
		.operations = &uninit_ops,
		.va = va,
		.frame = NULL, /* no frame for now 아직 프레임이 없습니다.*/
		.uninit = (struct uninit_page) {
			.init = init,
			.type = type,
			.aux = aux,
			.page_initializer = initializer,
		}
	};
}

/* Initalize the page on first fault 첫 번째 폴트 시 페이지를 초기화합니다.  */
static bool
uninit_initialize (struct page *page, void *kva) {
	struct uninit_page *uninit = &page->uninit;

	/* Fetch first, page_initialize may overwrite the values 값을 덮어씌우기 전에 fetch합니다. */
	vm_initializer *init = uninit->init;
	void *aux = uninit->aux;

	/* TODO: You may need to fix this function. 
	TODO: 이 함수를 수정해야 할 수도 있습니다.*/
	return uninit->page_initializer (page, uninit->type, kva) &&
		(init ? init (page, aux) : true);
}

/* Free the resources hold by uninit_page. Although most of pages are transmuted
to other page objects, it is possible to have uninit pages when the process
exit, which are never referenced during the execution.
PAGE will be freed by the caller.

uninit_page에 의해 보유된 리소스를 해제합니다. 대부분의 페이지는 다른 페이지 객체로 변환되지만,
프로세스가 종료될 때 참조되지 않는 uninit 페이지가 있는 경우가 있을 수 있습니다.
PAGE는 호출자에 의해 해제됩니다 */
static void
uninit_destroy (struct page *page) {
	struct uninit_page *uninit UNUSED = &page->uninit;
	/* TODO: Fill this function.
	 * TODO: If you don't have anything to do, just return. 
	 TODO: 이 함수를 구현합니다.
	TODO: 수행할 작업이 없다면 그냥 반환하세요.*/
}
