import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { WidgetsModule } from 'web-app-framework';

import { DsbdwidgetheaderComponent } from './dsbdwidgetheader.component';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';

describe('DsbdwidgetheaderComponent', () => {
  let component: DsbdwidgetheaderComponent;
  let fixture: ComponentFixture<DsbdwidgetheaderComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [DsbdwidgetheaderComponent, PrettyDatePipe],
      imports: [
        WidgetsModule,
        MaterialdesignModule
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(DsbdwidgetheaderComponent);
    component = fixture.componentInstance;
    component.icon = {
      margin: {
        top: '5px',
        left: '5px'
      }
    };
    component.id = 'test_id';
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
