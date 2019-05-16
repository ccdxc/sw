import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { MaterialdesignModule } from '@lib/materialdesign.module';

import { ModalheaderComponent } from './modalheader.component';

describe('ModalheaderComponent', () => {
  let component: ModalheaderComponent;
  let fixture: ComponentFixture<ModalheaderComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [ModalheaderComponent],
      imports: [MaterialdesignModule, ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(ModalheaderComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
