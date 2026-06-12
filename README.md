# SecTop
Monitor de rede e segurança para terminal (TUI) focado em performance e análise tática de conexões e detecção de anomalias em tempo real, para linux.

> **Nota do Desenvolvedor:** Este projeto está em fase ativa de desenvolvimento. Além de construir uma ferramenta útil para a comunidade, o propósito central deste repositório é servir como um laboratório prático (e uma excelente desculpa) para o estudo profundo de C++ de baixo nível, interações diretas com o Kernel do Linux e arquitetura de sistemas operacionais.


O SecTop foi desenhado para preencher a lacuna entre monitores de recursos genéricos (como `htop` ou `btop`) e ferramentas pesadas de análise de pacotes.

Ele fornece uma visão imediata dos dados da rede do sistema mapeando conexões ativas diretamente para os processos responsáveis com o mínimo de overhead no consumo de CPU.

## Arquitetura e Filosofia
Para garantir alta performance e baixa latência, o SecTop não executa chamadas de sistema custosas ou parseamento de texto de utilitários externos (como `ss` ou `netstat`). 

A extração de dados é feita de forma nativa e em baixo nível:
* **Coleta de Conexões:** Comunicação direta com o subsistema do kernel via sockets Netlink (`NETLINK_INET_DIAG`), recebendo structs binárias prontas.
* **Resolução de Processos:** Mapeamento reverso consultando o VFS do Linux (`/proc/[pid]/fd`) para cruzar inodes de rede com PIDs e executáveis.
* **Interface Gráfica:** Renderização TUI assíncrona utilizando a biblioteca FTXUI.

## Funcionalidades

- Monitoramento de conexões TCP/UDP em tempo real (IPv4/IPv6).
- Mapeamento instantâneo de portas em estado `LISTEN` com seus respectivos processos e PIDs.
- Baixo consumo de recursos, ideal para rodar continuamente em *tiling window managers* ou servidores de borda.
- Interface TUI responsiva e gerenciada de forma assíncrona.
- *(Em desenvolvimento)* Detecção heurística de padrões anômalos (Port Scans, picos de conexões).

## Requisitos do Sistema

O SecTop é construído especificamente para o ecossistema Linux.

* Sistema Operacional: Linux (Kernel 3.1+ com suporte a Netlink socket diag)
* Compilador: GCC 9+ ou Clang 10+ (Suporte a C++17)
* Dependências de Build: `cmake`, `make`

## Compilação e Instalação

O projeto utiliza o CMake como sistema de build principal. Para clonar e compilar a ferramenta a partir da fonte:

```bash
# Clone o repositório
git clone [https://github.com/SEU_USUARIO/sectop.git](https://github.com/SEU_USUARIO/sectop.git)
cd sectop

# Crie o diretório de build
mkdir build && cd build

# Configure e compile o projeto
cmake ..
make -j$(nproc)
