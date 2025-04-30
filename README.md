
<h1 align="center">
TDE - Ponte de Processos e Threads
</h1>

<p align="center">
  <a href="#"><img src="https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white"/></a>
  
</p>


## 🌉 Descrição do Projeto

Este projeto simula a travessia de veículos em uma ponte utilizando **processos (via `fork`)** e **threads POSIX (`pthreads`)** em linguagem C.
A ponte possui capacidade limitada e regras específicas para garantir a segurança e evitar colisões entre os veículos que trafegam em sentidos opostos.

---

## ⚙️ Funcionalidades

- Criação de múltiplos processos e threads para representar veículos
- Controle de acesso à ponte com base em semáforos e mutexes
- Implementação de políticas para evitar deadlocks e starvation
- Logs detalhados das ações dos veículos (entrada, travessia e saída da ponte)

---

## 🛠️ Tecnologias Utilizadas

- Linguagem C
- POSIX Threads (pthreads)
- `fork()` para criação de processos
- Sistema Operacional Linux

---

### Passos para rodar o pthreads

1. Clone o repositório:

```bash
git clone https://github.com/RicardoMBregalda/tde-so-ponte.git
cd tde-so-ponte
```

2. Compile o código:

```bash
gcc -pthread -o ponte pthread.c
```

3. Execute o programa:

```bash
./ponte
```

### Passos para rodar o fork

1. Clone o repositório:

```bash
git clone https://github.com/RicardoMBregalda/tde-so-ponte.git
cd tde-so-ponte
```

2. Compile o código:

```bash
gcc -o fork fork.c
```

3. Execute o programa:

```bash
./fork
```

---

## 👨‍💻 Desenvolvedores

- [Ricardo Bregalda](https://github.com/RicardoMBregalda)
- [Matheus Tregnago](https://github.com/matregnago)
