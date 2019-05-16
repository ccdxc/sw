import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { WidgetsModule } from 'web-app-framework';
import { PrimengModule } from '@app/lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';

import { SoftwareversionComponent } from './softwareversion.component';
import { DsbdwidgetheaderComponent } from '@app/components/shared/dsbdwidgetheader/dsbdwidgetheader.component';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';

describe('SoftwareversionComponent', () => {
  let component: SoftwareversionComponent;
  let fixture: ComponentFixture<SoftwareversionComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [SoftwareversionComponent, DsbdwidgetheaderComponent, PrettyDatePipe],
      imports: [
        WidgetsModule,
        PrimengModule,
        MaterialdesignModule,
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(SoftwareversionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
