import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { TableheaderComponent } from './tableheader.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

describe('TableheaderComponent', () => {
  let component: TableheaderComponent;
  let fixture: ComponentFixture<TableheaderComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [TableheaderComponent],
      imports: [
        MaterialdesignModule
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
