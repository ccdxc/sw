import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { ModalcontentComponent } from './modalcontent.component';

describe('ModalcontentComponent', () => {
  let component: ModalcontentComponent;
  let fixture: ComponentFixture<ModalcontentComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [ModalcontentComponent],
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(ModalcontentComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
