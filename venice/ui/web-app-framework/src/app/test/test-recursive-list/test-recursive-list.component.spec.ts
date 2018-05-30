import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { TestRecursiveListComponent } from './test-recursive-list.component';

import { WidgetsModule } from '@modules/widgets/widgets.module';
describe('TestRecursiveListComponent', () => {
  let component: TestRecursiveListComponent;
  let fixture: ComponentFixture<TestRecursiveListComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [TestRecursiveListComponent],
      imports: [WidgetsModule]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TestRecursiveListComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
