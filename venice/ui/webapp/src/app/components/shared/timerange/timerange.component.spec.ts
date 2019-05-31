import { ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { PrimengModule } from '@app/lib/primeng.module';
import { configureTestSuite } from 'ng-bullet';
import { TimeRangeComponent } from './timerange.component';
import { PrettyDatePipe } from '../Pipes/PrettyDate.pipe';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { NO_ERRORS_SCHEMA } from '@angular/core';


describe('TimerangeComponent', () => {
  let component: TimeRangeComponent;
  let fixture: ComponentFixture<TimeRangeComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [TimeRangeComponent, PrettyDatePipe],
      imports: [
        RouterTestingModule,
        PrimengModule,
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
      ],
      schemas: [NO_ERRORS_SCHEMA],
      providers: []
    });
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(TimeRangeComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
