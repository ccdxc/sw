import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { FormGroup, FormsModule } from '@angular/forms';

import { TableheaderComponent } from './tableheader.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { PrettyDatePipe } from '../Pipes/PrettyDate.pipe';

describe('TableheaderComponent', () => {
  let component: TableheaderComponent;
  let fixture: ComponentFixture<TableheaderComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [TableheaderComponent, PrettyDatePipe],
      imports: [
        MaterialdesignModule,
        PrimengModule,
        FormsModule
      ]
    });
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(TableheaderComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
