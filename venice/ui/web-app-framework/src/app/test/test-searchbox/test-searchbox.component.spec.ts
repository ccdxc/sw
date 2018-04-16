import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { FormsModule } from '@angular/forms';

import { TestSearchboxComponent } from './test-searchbox.component';
import { WidgetsModule } from '@modules/widgets/widgets.module';

describe('TestSearchboxComponent', () => {
  let component: TestSearchboxComponent;
  let fixture: ComponentFixture<TestSearchboxComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [
        TestSearchboxComponent
      ],
      imports: [
        WidgetsModule,
        FormsModule
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TestSearchboxComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
