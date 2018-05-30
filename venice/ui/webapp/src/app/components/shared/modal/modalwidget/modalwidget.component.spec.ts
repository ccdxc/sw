import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ModalwidgetComponent } from './modalwidget.component';

describe('ModalwidgetComponent', () => {
  let component: ModalwidgetComponent;
  let fixture: ComponentFixture<ModalwidgetComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ModalwidgetComponent],
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ModalwidgetComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
