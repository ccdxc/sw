import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { TestplotlyComponent } from './testplotly.component';
import { WidgetsModule } from '@modules/widgets/widgets.module';
declare var Plotly: any;

describe('TestplotlyComponent', () => {
  let component: TestplotlyComponent;
  let fixture: ComponentFixture<TestplotlyComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [TestplotlyComponent],
      imports: [WidgetsModule]
    });
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TestplotlyComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
