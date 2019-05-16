import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { MaterialdesignModule } from '@lib/materialdesign.module';
import { ModalitemComponent } from './modalitem.component';

describe('ModalitemComponent', () => {
  let component: ModalitemComponent;
  let fixture: ComponentFixture<ModalitemComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [ModalitemComponent],
      imports: [MaterialdesignModule, ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(ModalitemComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
