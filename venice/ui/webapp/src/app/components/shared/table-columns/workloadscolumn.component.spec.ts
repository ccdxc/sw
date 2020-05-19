import { ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { WorkloadsColumnComponent } from './workloadscolumn.component';
import { PrimengModule } from '@app/lib/primeng.module';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { HttpClientTestingModule } from '@angular/common/http/testing';

describe('WorkloadsColumnComponent', () => {
  let component: WorkloadsColumnComponent;
  let fixture: ComponentFixture<WorkloadsColumnComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [WorkloadsColumnComponent],
      imports: [
        RouterTestingModule,
        HttpClientTestingModule,
        PrimengModule,
        FormsModule,
        MaterialdesignModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
      ],
      providers: []
    });
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(WorkloadsColumnComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
