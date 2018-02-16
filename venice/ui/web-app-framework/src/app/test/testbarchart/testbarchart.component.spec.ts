import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import {WidgetsModule} from '@modules/widgets/widgets.module';
import { TestbarchartComponent } from './testbarchart.component';

describe('TestbarchartComponent', () => {
  let component: TestbarchartComponent;
  let fixture: ComponentFixture<TestbarchartComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ TestbarchartComponent], // ,BarchartCoponent]
      imports: [WidgetsModule]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TestbarchartComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
